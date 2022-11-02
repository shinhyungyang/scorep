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

/* Add a call to this function  from SCOREP library in code to count Basic Blocks */
std::string aScorepInstrFunc        = "SCOREP_Timer_IncrementLogical_multi";

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
    /* Comment: for each #pragma, 2 compiler directives are generated: .omp_outlined. & .omp_outlined._debug__.
                .omp_outlined. calls .omp_outlined._debug__. and each of them would have an index to refer to the
                corresponding #pragma */
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

    bool is_POMP = func_name_as_string.find( "POMP2" ) != std::string::npos;
    bool is_pomp = func_name_as_string.find( "pomp2" ) != std::string::npos;
    bool is_Pomp = func_name_as_string.find( "Pomp2" ) != std::string::npos;


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
is_pmpi_function( const Function& func,
                  std::string*    error = nullptr )
{

    std::string func_name_as_string = func.getName().str();

    bool is_PMPI = func_name_as_string.find( "PMPI" ) != std::string::npos;

    if ( is_PMPI )
    {
        *error = "is PMPI";

        return true;
    }
    return false;
}

static bool
is_scorep_function( const Function& func,
                  std::string*    error = nullptr )
{

    std::string func_name_as_string = func.getName().str();

    bool is_SCOREP = func_name_as_string.find( "SCOREP" ) != std::string::npos;
    bool is_scorep = func_name_as_string.find( "scorep" ) != std::string::npos;

    if (    is_SCOREP
         || is_scorep )
    {
        *error = "is SCOREP function";

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

/* Comment: Only user code shall be instrumented, discard empty, POMP2 inserted code or artificial funcs */
/* Comment: Due to how Score-P works, MPI calls should be discarded (they call a Score-P wrapper instead of
            MPI lib functions), and we should only instrument the PMPI functions, but this is not possible
            because here we only instrument the user code.
            This will be extra check in case any of PMPI functions are inlined and
            gets instrumened. */
static bool
is_instrumentable( const Function& func )
{
    std::string error;

    if (    has_empty_body( func, &error )
         || is_pomp_function( func, &error )
         || is_pmpi_function( func, &error )
         || is_scorep_function( func, &error )
         || is_artificial( func, &error ) )
    {
        /* if ( ClScorepPassVerbose >= 1 )
        {
            errs() << "Ignore: " + func.getName().str() + " reason: " + error + "\n";
        } */
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

  /*!
   *  Find/declare a function taking a double `i64*` argument with a void return
   *  type suitable for making a call to in IR. This is used to get references
   *  to the SCOREP profiling function symbols.
   *
   * \param funcname The name of the function
   * \param ctx The LLVMContext
   * \param mdl The Module in which the function will be used
   */
static FunctionCallee getVoidFuncMultiParam(StringRef funcname, LLVMContext &context, Module *module) {

    // Void return type
    Type *retTy = Type::getVoidTy(context);

    // int64 argument
    Type *argTw = Type::getInt64Ty(context);

    // int64 argument
    Type *argTz = Type::getInt64Ty(context);

    SmallVector<Type *, 2> paramTys {argTw, argTz};

    // Third param to `get` is `isVarArg`.  It's not documented, but might have
    // to do with variadic functions?
    FunctionType *funcTy = FunctionType::get(retTy, paramTys, false);
    return module->getOrInsertFunction(funcname, funcTy);
}

struct BasicBlockPass : public FunctionPass {
    static char ID;
    BasicBlockPass() : FunctionPass(ID) {}

    virtual bool runOnFunction(Function &func) {

        /* Comment: First basic block inside #pragma omp */
        bool firstBBinOMP    = true;
        /* Comment: Makeup for skipped basic block count in #pragma omp */
        bool makeUpForSkipBB = true;

        if ( !is_instrumentable( func ) )
        {
            /* Comment: Function is not modified */
            return false;
        }

        auto &context = func.getContext();
        auto *module  = func.getParent();

        FunctionCallee
            onCallFuncInstrBB   = getVoidFunc(aScorepInstrFuncBBCnt, context, module);
        FunctionCallee
            onCallFuncInstrStmt = getVoidFunc(aScorepInstrFuncStmtCnt, context, module);
        FunctionCallee
            onCallFuncInstr     = getVoidFuncMultiParam(aScorepInstrFunc, context, module);

        bool mutated                    = false;
        //int bbCount    = 0; /* for debugging only */
        int bbIncValue                  = 1;
        int stmtIncValue                = 0;
        int stmtIncValueMakeUpForSkipBB = 0;

        IntegerType *Int64 = Type::getInt64Ty(context);

        /* Comment: first valid instruction to insert instructions before from LLVM aspect */
        Instruction * bbBeginMarker;

        for (auto &block : func) {

            /* TODO: one corner case, if only one basic block is inside the omp_outlined, current
            implementation misses counting it.
            Check that and add an extra call to SCOREP_TimerInc at the end of this block to SCOREP_TimerInc
            Note that normal check on has_empty_body function doesn't work with omp_outlined
            */
            /*
            LLVMModuleRef M = llvm_load_module(false, false); // this is an internal func
            LLVMValueRef f;
            f = LLVMGetFirstFunction(M)
            //auto funcValRef    = LLVMValueRef(func);
            auto BasicBlockCount = LLVMCountBasicBlocks(f);
            if (BasicBlockCount == 0)
            {
                // OMP outline func doesnt contain basic blocks
            }
            */

            /* Comment: Instrument the block only if it is not an Exception Handler, else skip
                        address inside Exception Handler results in memory violation error. */
            if (!(block.getFirstNonPHI()->isEHPad()))
            {
                // bbcount++; // for debugging only
                /* Comment: get the first address in the basic block which is after phi instruction
                            if all instructions are PHI, 0 is returned. */
                bbBeginMarker = block.getFirstNonPHI();

                if (bbBeginMarker)
                {
                    /* Comment: Can't instrument .omp_outlined. or .omp_outlined._debug__ sections directly,
                                instrumentation function must be inserted after call to POMP2_Parallel_begin()
                                func is executed in the basic block so that to have each thread's TPD initialzed.
                                This check is added to bypass instrumenting the outermost basic block inside
                                #pragma, and later make up for that bypassed basic block by adding two extra
                                basic block counts (one for omp_outlined and one for omp_outlined._debug)
                                in the following inner basic block. */
                    if (is_openmp_function(func) && firstBBinOMP)
                    {
                        firstBBinOMP = false;
                        /* Comment: Need to count skipped number of instructions in that skipped basic block */
                        stmtIncValueMakeUpForSkipBB = std::distance(block.begin(), block.end());
                        /* Comment: Skip first iteration of for loop, i.e: don't instrument first block */
                        continue;
                    }

                    IRBuilder<> builder(bbBeginMarker);

                    /* Comment: Update increment values with skipped values once we are
                                in the first inner level basic block.
                                Here we are sure that POMP2_Parallel_begin is called
                                and that the thread's TPD is initialized. */
                    if (is_openmp_function(func) && !firstBBinOMP && makeUpForSkipBB)
                    {
                        bbIncValue      = 3; // more on that value in notes
                        stmtIncValue    = stmtIncValue + stmtIncValueMakeUpForSkipBB;
                        makeUpForSkipBB = false;
                    }
                    else
                    {
                        /* Comment: Default value for BB count unless there's a following update. */
                        bbIncValue = 1;

                        /* Comment: Number of statments inside a basic block */
                        stmtIncValue = std::distance(block.begin(), block.end());
                    }

                    Value *varArguments_bb = ConstantInt::get(Int64, bbIncValue);
                    SmallVector<Value *, 1> args_bb{varArguments_bb};

                    Value *varArguments_stmt = ConstantInt::get(Int64, stmtIncValue);
                    SmallVector<Value *, 1> args_stmt{varArguments_stmt};

                    /* Insert a call to instrumentation function */
                    //builder.CreateCall(onCallFuncInstrBB, args_bb);
                    //builder.CreateCall(onCallFuncInstrStmt, args_stmt);

                    SmallVector<Value *, 2> args{varArguments_bb, varArguments_stmt};
                    builder.CreateCall(onCallFuncInstr, args);

                    mutated = true;
                }
            }
        }

        /* Comment: We need micro instrumentation to update sync. points relatively
                    fast, so use block Instr for Statmt. count
                    else we can use the following method to update logical_stmt
                    count at end of each function:
                    func.getInstructionCount() */

        return mutated;
    }
};

} /* namespace */

char BasicBlockPass::ID = 0;

/* Comment: First argument here is used as a command line for the pass name when using opt (LLVM optimizer) */
static RegisterPass<BasicBlockPass> X ( "BasicBlockCountPass",
                                        "Counts dynamic basic blocks",
                                        false,
                                        false);

static RegisterStandardPasses Y( PassManagerBuilder::EP_EarlyAsPossible,
                                [](const PassManagerBuilder &Builder,
                                legacy::PassManagerBase &PM) { PM.add(new BasicBlockPass()); });

// Automatically enable the pass.
// http://adriansampson.net/blog/clangpass.html
/* static void registerBasicBlockPass(  const PassManagerBuilder &,
                                        legacy::PassManagerBase &PM )
{
    PM.add(new BasicBlockPass());
}

static RegisterStandardPasses RegisterMyPass(   PassManagerBuilder::EP_EarlyAsPossible,
                                                registerBasicBlockPass );
*/
