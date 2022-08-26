#include "llvm/Pass.h"
#include "llvm/ADT/Statistic.h"
#include "llvm/ADT/StringSet.h"
#include "llvm/ADT/SmallVector.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/Constants.h"
#include "llvm/IR/Function.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Support/CommandLine.h"
#include "llvm/IR/Module.h"


// Need these to use Sampson's registration technique
#include "llvm/Transforms/IPO/PassManagerBuilder.h"
#include "llvm/IR/LegacyPassManager.h"

#include <fstream>


using namespace llvm;

namespace
{

/* Add a call to this function  from SCOREP library in code to count Basic Blocks */
std::string aScorepInstrFuncBBCnt   = "SCOREP_Timer_IncrementLogical";
/* Add a call to this function  from SCOREP library in code to count statements */
std::string aScorepInstrFuncStmtCnt = "SCOREP_Timer_IncrementLogical_StmtCnt";

static bool
has_empty_body( const Function& func,
                std::string*    error = nullptr )
{
    unsigned long size = func.getEntryBlock().getInstList().size();

    if ( func.getReturnType() == Type::getVoidTy( func.getContext() ) && size == 1 )
    {
        if ( error != nullptr )
        {
            *error = "is empty";
        }
        return true;
    }
    return false;
}

static bool
is_openmp_function( const Function& func,
                    std::string*    error = nullptr )
{
    std::string omp_outlined_func_name( ".omp_outlined." );
    std::string omp_func_name( ".omp." );
    std::string nondebug_wrapper_func_name( ".nondebug_wrapper." );

    std::string func_name_as_string = func.getName().str();
    if ( !func_name_as_string.compare( 0, omp_outlined_func_name.size(), omp_outlined_func_name ) ||
         !func_name_as_string.compare( 0, omp_func_name.size(), omp_func_name ) ||
         !func_name_as_string.compare( 0, nondebug_wrapper_func_name.size(), nondebug_wrapper_func_name ) )
    {
        if ( error != nullptr )
        {
            *error = "is OMP outlined";
        }
        return true;
    }
    return false;
}

static bool
is_pomp_function( const Function& func,
                  std::string*    error = nullptr )
{

    std::string func_name_as_string = func.getName().str();

    bool is_POMP = func_name_as_string.find( "POMP" ) != std::string::npos;
    bool is_pomp = func_name_as_string.find( "pomp" ) != std::string::npos;
    bool is_Pomp = func_name_as_string.find( "Pomp" ) != std::string::npos;


    if (    is_POMP
         || is_pomp
         || is_Pomp )
    {
        *error = "is POMP";

        return true;
    }
    return false;
}

static bool
is_artificial( const Function& func,
               std::string*    error = nullptr )
{
    std::string func_name_as_string = func.getName().str();
    bool        is_cxa              = func_name_as_string.find( "__cxx_" ) != std::string::npos;
    bool        is_global           = func_name_as_string.find( "_GLOBAL__" ) != std::string::npos;

    if ( is_cxa || is_global )
    {
        if ( error != nullptr )
        {
            *error = "is artificial";
        }
        return true;
    }
    return false;
}

static bool
is_instrumentable( const Function& func )
{
    std::string error;
    if ( has_empty_body( func, &error )
         || is_openmp_function( func, &error )
         || is_artificial( func, &error ) )
    {
        //if ( ClScorepPassVerbose >= 1 )
        //{
        //errs() << "Ignore: " + func.getName().str() + " reason: " + error + "\n";
        //}
        return false;
    }
    return true;
}

  /*!
   *  Find/declare a function taking a single `i64*` argument with a void return
   *  type suitable for making a call to in IR. This is used to get references
   *  to the SCOREP profiling function symbols.
   *
   * \param funcname The name of the function
   * \param ctx The LLVMContext
   * \param mdl The Module in which the function will be used
   */
static FunctionCallee getVoidFunc(StringRef funcname, LLVMContext &context, Module *module) {

    // Void return type
    Type *retTy = Type::getVoidTy(context);

    // int64 argument
    Type *argTz = Type::getInt64Ty(context);

    SmallVector<Type *, 1> paramTys {argTz};

    // Third param to `get` is `isVarArg`.  It's not documented, but might have
    // to do with variadic functions?
    FunctionType *funcTy = FunctionType::get(retTy, paramTys, false);
    return module->getOrInsertFunction(funcname, funcTy);
}

struct SkeletonPass : public FunctionPass {
    static char ID;
    SkeletonPass() : FunctionPass(ID) {}

    virtual bool runOnFunction(Function &func) {

        if ( !is_instrumentable( func ) )
        {
            /* function is not modified */
            return false;
        }
        auto &context = func.getContext();
        auto *module  = func.getParent();
        FunctionCallee
            onCallFuncInstrBB   = getVoidFunc(aScorepInstrFuncBBCnt, context, module);
        FunctionCallee
            onCallFuncInstrStmt = getVoidFunc(aScorepInstrFuncStmtCnt, context, module);

        bool mutated   = false;
        //int bbCount    = 0; /* for debugging only */
        int bbIncValue = 1;
        int stmtIncValue = 0;

        IntegerType *Int64 = Type::getInt64Ty(context);

        Instruction * bbBeginMarker; // first valid instruction to insert instructions before

        for (auto &block : func) {
            //bbCount++; /* for debugging only */
            /* instrument the block only if it is not an Exception Handler, else skip */
            /* address inside Exception Handler results in memory violation error */
            if (!(block.getFirstNonPHI()->isEHPad()))
            {
                /* get the first address in the basic block which is after phi instruction */
                /* if all instructions are PHI, 0 is returned */
                bbBeginMarker = block.getFirstNonPHI();
                if (bbBeginMarker)
                {
                    IRBuilder<> builder(bbBeginMarker);

                    Value *varArguments_bb = ConstantInt::get(Int64, bbIncValue);
                    SmallVector<Value *, 1> args_bb{varArguments_bb};

                    /* number of statments inside a block */
                    stmtIncValue = std::distance(block.begin(), block.end());

                    Value *varArguments_stmt = ConstantInt::get(Int64, stmtIncValue);
                    SmallVector<Value *, 1> args_stmt{varArguments_stmt};

                    /* Insert a call to instrumentation function */
                    builder.CreateCall(onCallFuncInstrBB, args_bb);
                    builder.CreateCall(onCallFuncInstrStmt, args_stmt);

                    mutated = true;
                }
            }
        }
        //errs() << "Basic Block count in function : " << func.getName() << " = " << bbCount << "\n"; /* for debugging only */
        /* We need micro instrumentation to update sync. points relatively fast, so use block Instr for Statmt. count */
        /* else we can use the following method to update logical_stmt count at end of each function. */
        //errs() << "Instructions count in function : " << func.getName() << " = " <<  func.getInstructionCount() << "\n";
        return mutated;
    }
};

} /* namespace */

char SkeletonPass::ID = 0;

// Automatically enable the pass.
// http://adriansampson.net/blog/clangpass.html
static void registerSkeletonPass(const PassManagerBuilder &,
                         legacy::PassManagerBase &PM) {
  PM.add(new SkeletonPass());
}
static RegisterStandardPasses
  RegisterMyPass(PassManagerBuilder::EP_EarlyAsPossible,
                 registerSkeletonPass);
