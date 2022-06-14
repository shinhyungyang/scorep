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

bool bDryRun                      = false;
std::string aScorepStartFunc      = "enter";
std::string aScorepStartFuncMulti = "SCOREP_Timer_SetLogical"; // SCOREP_Timer_SetLogical but use other api for omp for loops inside stream
std::string aScorepStopFunc       = "leave";

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
         //|| is_openmp_function( func, &error )
         || is_artificial( func, &error ) )
    {
        //if ( ClScorepPassVerbose >= 1 )
        //{
        errs() << "Ignore: " + func.getName().str() + " reason: " + error + "\n";
        //}
        return false;
    }
    return true;
}


static FunctionCallee getVoidFunc(StringRef funcname, LLVMContext &context, Module *module) {

    // Void return type
    Type *retTy = Type::getVoidTy(context);

    // TODO: _ndao: int64 is not compatible with uint64 (casted in Score-P)
    // int64 argument
    Type *argTz = Type::getInt64Ty(context);
    SmallVector<Type *, 1> paramTys{argTz};

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
            onCallFunc = getVoidFunc(aScorepStartFuncMulti, context, module);

        bool mutated = false;
        int bbCount  = 0;
        IntegerType *Int64;
        Int64 = Type::getInt64Ty(context);

        Instruction * bbBeginMarker; // first valid instruction to insert instructions before

        for (auto &block : func) {
            bbCount++;
            /* get the first address in the basic block which is after phi instruction */
            /* if all instructions are PHI, 0 is returned */
            bbBeginMarker = block.getFirstNonPHI();
            if (bbBeginMarker)
            {
                IRBuilder<> builder(bbBeginMarker);

                Value *varArguments = ConstantInt::get(Int64, bbCount);
                SmallVector<Value *, 1> args{varArguments};

                builder.CreateCall(onCallFunc, args);
                mutated = true;
            }
        }
        errs() << "Basic Block count in function : " << func.getName() << " = " << bbCount << "\n";
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
