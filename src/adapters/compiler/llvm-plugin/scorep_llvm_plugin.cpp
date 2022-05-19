/*
 * This file is part of the Score-P software (http://www.score-p.org)
 *
 * Copyright (c) 2017-2018,
 * Technische Universitaet Dresden, Germany
 *
 * This software may be modified and distributed under the terms of
 * a BSD-style license. See the COPYING file in the package base
 * directory for details.
 *
 */

/**
 * @file
 *
 * @brief  Implementation of a LLVM pass in order to instrument functions.
 *
 */

#include <config.h>

#include <tuple>

#include <llvm/Pass.h>
#include <llvm/IR/Function.h>
#include <llvm/Support/raw_ostream.h>
#include <llvm/IR/LegacyPassManager.h>
#include <llvm/IR/InstrTypes.h>
#include <llvm/Transforms/IPO/PassManagerBuilder.h>
#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/BasicBlock.h>
#include <llvm/Transforms/Utils/BasicBlockUtils.h>
#include <llvm/IR/Module.h>
#include <llvm/Transforms/Utils/Local.h>
#include <llvm/Analysis/EHPersonalities.h>
#include <llvm/IR/DebugInfo.h>

#if defined( LLVM_DEMANGLE )
#include <llvm/Demangle/Demangle.h> // since llvm 4.0
#endif /* LLVM_DEMANGLE */

#include <SCOREP_Filter.h>

using namespace llvm;


#if defined( GNU_DEMANGLE )
/* Declaration of external demangling function */
/* It is contained in "demangle.h" */
extern "C" char*
cplus_demangle( const char* mangled,
                int options );

/* cplus_demangle options */
#define SCOREP_COMPILER_DEMANGLE_NO_OPTS   0
#define SCOREP_COMPILER_DEMANGLE_PARAMS    ( 1 << 0 )  /* include function arguments */
#define SCOREP_COMPILER_DEMANGLE_ANSI      ( 1 << 1 )  /* include const, volatile, etc. */
#define SCOREP_COMPILER_DEMANGLE_VERBOSE   ( 1 << 3 )  /* include implementation details */
#define SCOREP_COMPILER_DEMANGLE_TYPES     ( 1 << 4 )  /* include type encodings */

/* Demangling style. */
static int scorep_compiler_demangle_style = SCOREP_COMPILER_DEMANGLE_PARAMS  |
                                            SCOREP_COMPILER_DEMANGLE_ANSI    |
                                            SCOREP_COMPILER_DEMANGLE_VERBOSE |
                                            SCOREP_COMPILER_DEMANGLE_TYPES;
#endif /* GNU_DEMANGLE */


/**
 * Command line option to provide path to an optional Score-P filter file
 */
static cl::opt<std::string> ClScorepPassFilterFile(
    "fplugin-arg-scorep_instrument_function-filter",
    cl::desc( "Path to Score-P filter file" ),
    cl::NotHidden,
    cl::init( "" ) );

/**
 * Command line option to enable verbose output
 */
static cl::opt<int> ClScorepPassVerbose(
    "fplugin-arg-scorep_instrument_function-verbosity",
    cl::desc( "Enable verbose output" ),
    cl::NotHidden,
    cl::init( 0 ) );

/**
 * The Score-P filtering file
 */
static SCOREP_Filter* scorep_instrument_filter = NULL;

namespace
{
using exit_point_list_t = std::vector<Instruction*>;
using call_instr_list_t = std::vector<CallInst*>;
using resume_block_t    = BasicBlock;
using exception_type_t  = AllocaInst;
using exception_ptr_t   = AllocaInst;

using scorep_function_t      = Constant;
using personality_function_t = Constant;

using exception_slots_t = struct
      {
          exception_ptr_t*  exception_ptr;
          exception_type_t* exception_type;
      };

using function_parsing_result_t = struct
      {
          exit_point_list_t exit_points;
          call_instr_list_t call_instr;
      };


#if defined( SCOREP_LLVM_PLUGIN_TARGET_VERSION ) && SCOREP_LLVM_PLUGIN_TARGET_VERSION < 4000

/*
 * Function changeToInvokeAndSplitBasicBlock() is not available in LLVM <4.0.
 * Therefore, we provide an own implementation of this function.
 */

static BasicBlock*
changeToInvokeAndSplitBasicBlock( CallInst*   CI,
                                  BasicBlock* UnwindEdge )
{
    BasicBlock* BB = CI->getParent();

    // Convert this function call into an invoke instruction.  First, split the
    // basic block.
    BasicBlock* Split =
        BB->splitBasicBlock( CI->getIterator(), CI->getName() + ".noexc" );

    // Delete the unconditional branch inserted by splitBasicBlock
    BB->getInstList().pop_back();

    // Create the new invoke instruction.
    SmallVector<Value*, 8>           InvokeArgs( CI->arg_begin(), CI->arg_end() );
    SmallVector<OperandBundleDef, 1> OpBundles;

    CI->getOperandBundlesAsDefs( OpBundles );

    // Note: we're round tripping operand bundles through memory here, and that
    // can potentially be avoided with a cleverer API design that we do not have
    // as of this time.

    InvokeInst* II = InvokeInst::Create( CI->getCalledValue(), Split, UnwindEdge,
                                         InvokeArgs, OpBundles, CI->getName(), BB );
    II->setDebugLoc( CI->getDebugLoc() );
    II->setCallingConv( CI->getCallingConv() );
    II->setAttributes( CI->getAttributes() );

    // Make sure that anything using the call now uses the invoke!  This also
    // updates the CallGraph if present, because it uses a WeakTrackingVH.
    CI->replaceAllUsesWith( II );

    // Delete the original call
    Split->getInstList().pop_front();
    return Split;
}

#endif /* if defined( SCOREP_LLVM_PLUGIN_TARGET_VERSION ) && SCOREP_LLVM_PLUGIN_TARGET_VERSION < 4000 */

/**
 * Checks whether function has an empty body.
 *
 * @param f         Function to check.
 * @param error     Pointer to error message.
 *
 * @return true, if function has an empty body
 *
 * @todo use built-in functions?
 */
static bool
has_empty_body( const Function& f,
                std::string*    error = nullptr )
{
    unsigned long size = f.getEntryBlock().getInstList().size();

    if ( f.getReturnType() == Type::getVoidTy( f.getContext() ) && size == 1 )
    {
        if ( error != nullptr )
        {
            *error = "is empty";
        }
        return true;
    }
    return false;
}

/**
 * Checks whether it is an OpenMP internal function based on its name.
 *
 * @param f         Function to check.
 * @param error     Pointer to error message.
 *
 * @return true, if functions is an OpenMP internal function
 */
static bool
is_openmp_function( const Function& f,
                    std::string*    error = nullptr )
{
    std::string omp_outlined_func_name( ".omp_outlined." );
    std::string omp_func_name( ".omp." );
    std::string nondebug_wrapper_func_name( ".nondebug_wrapper." );

    std::string func_name_as_string = f.getName().str();
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

/**
 * Checks whether the function is created artificially by the compiler.
 *
 * @param f         Function to check.
 * @param error     Pointer to error message.
 *
 * @return true, if function is artificial
 *
 * @todo better characteristic
 */
static bool
is_artificial( const Function& f,
               std::string*    error = nullptr )
{
    std::string func_name_as_string = f.getName().str();
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

/**
 * Checks whether the function is filtered by the Score-P filter file.
 *
 * @param f         Function to check.
 * @param error     Pointer to error message.
 *
 * @return true, if function is filtered
 */
static bool
is_scorep_filtered( const Function& f,
                    std::string*    error = nullptr )
{
    std::string func_name = f.getName().str();
    std::string file      = "";

    if ( auto sub = f.getSubprogram() )
    {
        file = sub->getFilename().str();
    }

    int              result = false;
    SCOREP_ErrorCode err    = SCOREP_Filter_Match( scorep_instrument_filter,
                                                   file.c_str(),
                                                   func_name.c_str(),
                                                   func_name.c_str(),
                                                   &result );
    if ( err == SCOREP_SUCCESS && result )
    {
        if ( error != nullptr )
        {
            *error = "is excluded by filter file";
        }
        return true;
    }
    return false;
}

/**
 * Checks whether a function can be instrumented.
 *
 * @param f         Function to check.
 *
 * @return true, if function can be instrumented
 */
static bool
is_instrumentable( const Function& f )
{
    std::string error;
    if ( has_empty_body( f, &error )
         || is_openmp_function( f, &error )
         || is_artificial( f, &error )
         || is_scorep_filtered( f, &error ) )
    {
        if ( ClScorepPassVerbose >= 1 )
        {
            errs() << "Ignore: " + f.getName().str() + " reason: " + error + "\n";
        }
        return false;
    }
    return true;
}

/**
 * Gets pretty function name from its canonical representation.
 *
 * @param funcname  Mangled region name.
 *
 * @return demangled function name
 */
static std::string
demangle( const std::string& funcname )
{
    std::string funcname_demangled = funcname;
    char*       demangle_tmp;

    #if defined( LLVM_DEMANGLE )
    demangle_tmp = itaniumDemangle( funcname.c_str(), NULL, NULL, NULL );
    if ( demangle_tmp != NULL )
    {
        funcname_demangled = std::string( demangle_tmp );
    }

    ::free( demangle_tmp );

    /* Avoid warning about unused variable*/
    ( void )scorep_compiler_demangle_style;

    #elif defined( GNU_DEMANGLE )
    /* use demangled name if possible */
    if ( scorep_compiler_demangle_style >= 0 )
    {
        demangle_tmp = cplus_demangle( funcname.c_str(),
                                       scorep_compiler_demangle_style );

        if ( demangle_tmp != NULL )
        {
            funcname_demangled = std::string( demangle_tmp );
        }
    }
    #endif  /* GNU_DEMANGLE */

    return funcname_demangled;
}

/**
 * Creates the struct type for region_descriptor and insert it to LLVM IR.
 *
 * @param m         Module in which context struct will be inserted.
 *
 * @warning do not change order and insert new elements only at the end
 */
static StructType*
create_struct_type( Module* m )
{
    LLVMContext& ctx = m->getContext();

    auto* i32_type     = Type::getInt32Ty( ctx );
    auto* i32_ptr_type = PointerType::get( i32_type, 0 );
    auto* i8_ptr_type  = PointerType::getInt8PtrTy( ctx );


    StructType*              descriptor = StructType::create( ctx, "struct.scorep_region_description" );
    std::vector<llvm::Type*> params;
    params.push_back( i32_ptr_type );                   // handle ref
    params.push_back( i8_ptr_type );                    // name
    params.push_back( i8_ptr_type );                    // canonical name
    params.push_back( i8_ptr_type );                    // file
    params.push_back( i32_type );                       // begin lno
    params.push_back( i32_type );                       // end lno
    params.push_back( i32_type );                       // flags

    descriptor->setBody( params );

    return descriptor;
}


/**
 * Creates a global string variable.
 *
 * @param f         Function.
 * @param value     String content.
 * @param name      Variable's name you see in IR.
 * @return
 */
static GlobalVariable*
create_global_string( Function&          f,
                      const std::string& value,
                      const std::string& name = "" )
{
    auto        string   = ConstantDataArray::getString( f.getContext(), value );
    std::string var_name = "";
    var_name = f.getName().str() + "." + name;

    GlobalVariable* global = new GlobalVariable( *f.getParent(),
                                                 string->getType(),
                                                 true,
                                                 GlobalValue::LinkageTypes::PrivateLinkage,
                                                 string,
                                                 var_name
                                                 );
    global->setAlignment( 1 );
    return global;
}

/**
 * Gets the last instruction of the last basic block.
 *
 * @param f         Function to check.
 *
 * @return last instruction of the last basic block
 */
static Instruction&
get_function_end( Function& f )
{
    auto         basic_block_iter = --( f.end() );
    BasicBlock&  last_basic_block = *basic_block_iter;
    auto         instruction_iter = last_basic_block.rbegin();
    Instruction& last_instruction = *instruction_iter;

    // insert Score-P exit region
    return last_instruction;
}

/**
 * Gets meta data about a function.
 *
 * @param f         Function to check.
 *
 * @return tuple of ( begin line number, end line number, file name )
 */
static std::tuple<uint32_t, uint32_t, std::string>
get_meta_data( Function& f )
{
    uint32_t     begin_lno       = 0;
    uint32_t     end_lno         = 0;
    std::string  file            = "";
    Instruction& end_instruction = get_function_end( f );

    /* get begin line number and file name from functions node */
    if ( auto sub = f.getSubprogram() )
    {
        begin_lno = sub->getLine();
        file      = sub->getFilename().str();
    }

    /* end line number is not included in functions meta data.
     * therefore, try to get line number from the
     * last instruction (return) which is function's end */
    if ( DILocation* Loc = end_instruction.getDebugLoc() )
    {
        end_lno = Loc->getLine();
    }

    return std::make_tuple( begin_lno, end_lno, file );
}


/**
 * Creates an empty clean-up landing pad.
 *
 * @param f                 Function to check.
 * @param resumeBlock       Block of instructions executed in order to resume after exception.
 * @param exceptionSlots    Pointer and type of the exception.
 *
 * @return The basic block containing landing pad instructions.
 */
static BasicBlock*
create_landingpad( Function&         f,
                   BasicBlock*       resumeBlock,
                   exception_slots_t exceptionSlots )
{
    BasicBlock* bb = BasicBlock::Create( f.getParent()->getContext(), "landing_pad_block", &f, resumeBlock );

    IRBuilder<> builder( bb );

    LandingPadInst* pad = builder.CreateLandingPad(
        StructType::get( f.getContext(), { Type::getInt8PtrTy( f.getContext() ), Type::getInt32Ty( f.getContext() ) } ),
        0, "finally" );
    pad->setCleanup( true );

    auto extract1 = builder.CreateExtractValue( pad, 0 );
    builder.CreateStore( extract1, exceptionSlots.exception_ptr );
    auto extract2 = builder.CreateExtractValue( pad, 1 );
    builder.CreateStore( extract2, exceptionSlots.exception_type );

    builder.CreateBr( resumeBlock );

    return bb;
}
/**
 * Creates an allocation instruction in the function's begin
 *
 * @param f         Function to process.
 * @param type      Data type allocation.
 * @param initWith  Constant initialization value.
 * @param name      Instruction name for debugging purpose.
 *
 * @return Allocation instruction
 */
static AllocaInst*
create_entry_block_alloca( Function&          f,
                           llvm::Type*        type,
                           Constant*          initWith = 0,
                           const std::string& name = "" )
{
    BasicBlock& bb = f.getEntryBlock();
    IRBuilder<> builder( &bb, bb.begin() );

    AllocaInst* alloc = builder.CreateAlloca( type, 0, name );

    if ( initWith )
    {
        builder.CreateStore( initWith, alloc );
    }
    return alloc;
}

/**
 * Creates a resume block.
 *
 * @param f         Function to process.
 *
 * @return Resume block
 */
static std::tuple<resume_block_t*, exception_slots_t>
create_unwind_resume_block( Function& f )
{
    exception_slots_t exception_slots;

    exception_slots.exception_ptr  = create_entry_block_alloca( f, Type::getInt8PtrTy( f.getContext() ), 0, "exn.slot" );
    exception_slots.exception_type = create_entry_block_alloca( f, Type::getInt32Ty( f.getContext() ), 0, "ehselector" );


    BasicBlock* bb = BasicBlock::Create( f.getParent()->getContext(), "/reme", &f );
    IRBuilder<> builder( bb );

    Value* exception_ptr_val  = builder.CreateLoad( exception_slots.exception_ptr );
    Value* exception_type_val = builder.CreateLoad( exception_slots.exception_type );

    Type* lpad_type = StructType::get( f.getContext(),
                                       { Type::getInt8PtrTy( f.getContext() ), Type::getInt32Ty( f.getContext() ) } );
    Value* lpad_val = UndefValue::get( lpad_type );
    lpad_val = builder.CreateInsertValue( lpad_val, exception_ptr_val, 0, "lpad.val" );
    lpad_val = builder.CreateInsertValue( lpad_val, exception_type_val, 1, "lpad.val" );
    builder.CreateResume( lpad_val );

    return std::make_tuple( bb, exception_slots );
}

/**
 * Parses the function and returns a vector with all exit points, a vector with all call instructions which need to be
 * changed to invoke instructions, and a tuple of the resume block and the corresponding exception allocations
 * if exception handling is already present.
 *
 * @param f         Function to process.
 *
 * @return Data structure with information about exit and instrumentation points as well as exception handling
 */
static function_parsing_result_t
parse_function( Function& f )
{
    function_parsing_result_t parse_result;
    for ( BasicBlock& bb : f )
    {
        for ( Instruction& inst : bb )
        {
            if ( isa<ReturnInst>( &inst ) )
            {
                parse_result.exit_points.push_back( &inst );
            }
            // add call instructions to a vector to change them later to invoke instructions
            else if ( isa<CallInst>( &inst ) )
            {
                auto* call = cast<CallInst>( &inst );

                bool is_delete_operator = call->getCalledFunction()->getName().find( "ZdlPv" ) != std::string::npos;

                bool is_new_operator = call->getCalledFunction()->getName().find( "Znwm" ) != std::string::npos;

                bool is_llvm_function = call->getCalledFunction()->getName().find( "llvm." ) != std::string::npos;

                /* instrument only __cxa_throw (to get the scorep exit call also function which throws the exception)
                 * and no other __cxa_* functions */
                bool is_cxa       = call->getCalledFunction()->getName().find( "__cxa_" ) != std::string::npos;
                bool is_cxa_throw = call->getCalledFunction()->getName().find( "__cxa_throw" ) != std::string::npos;

                // TODO filter noexcept functions
                if ( !is_delete_operator &&
                     !is_new_operator &&
                     !is_llvm_function &&
                     ( !is_cxa || is_cxa_throw ) )
                {
                    parse_result.call_instr.push_back( call );
                }
            }
            else if ( isa<ResumeInst>( &inst ) )
            {
                parse_result.exit_points.push_back( bb.getFirstNonPHI() );
            }

            // even if there is no matching catch code, there should be a scorep_exit call
            else if ( isa<LandingPadInst> ( &inst ) )
            {
                auto land = cast<LandingPadInst> ( &inst );
                if ( !land->isCleanup() )
                {
                    land->setCleanup( true );
                }
            }
        }
    }
    return parse_result;
}


/**
 * Determines the instruction where the instrumentation of a 'function begin' needs to be inserted.
 *
 * @param f         Function to process.
 *
 * @return The first call instruction (if it is present), otherwise the last instruction of the first basic block.
 */
static Instruction&
get_insert_begin( Function& f )
{
    //Get the last instruction of the first basic block or the first call instruction in it.
    auto        basic_block_iter  = f.begin();
    BasicBlock& first_basic_block = *basic_block_iter;
    for ( auto& I : first_basic_block )
    {
        if ( !isa<AllocaInst>( &I ) )
        {
            return I;
        }
    }

    auto         instruction_iter = first_basic_block.rbegin();
    Instruction& instruction      = *instruction_iter;
    return instruction;
}


struct ScorepPass : public FunctionPass
{
    static char ID;

    ScorepPass() : FunctionPass( ID )
    {
        if ( ClScorepPassFilterFile.length() > 0 )
        {
            scorep_instrument_filter = SCOREP_Filter_New();
            SCOREP_ErrorCode status = SCOREP_Filter_ParseFile( scorep_instrument_filter, ClScorepPassFilterFile.c_str() );

            if ( status != SCOREP_SUCCESS )
            {
                errs() << "Couldn't load filter file " << ClScorepPassFilterFile.c_str() << ".\n";
                return;
            }
        }
    }

#if defined( SCOREP_LLVM_PLUGIN_TARGET_VERSION ) && SCOREP_LLVM_PLUGIN_TARGET_VERSION < 4000
    virtual const char*
    getPassName() const override
    {
        return "Score-P instrumentation";
    }
#else
    virtual StringRef
    getPassName() const override
    {
        return "Score-P instrumentation";
    }
#endif  /* if defined( SCOREP_LLVM_PLUGIN_TARGET_VERSION ) && SCOREP_LLVM_PLUGIN_TARGET_VERSION < 4000 */

    /**
     * This function is invoked for each function in the LLVM IR.
     * It implements to instrumentation of the current function
     * by calls to the Score-P measurement infrastructure.
     *
     * @param f         Function to process.
     *
     * @return True to indicate that the function was modified
     */
    virtual bool
    runOnFunction( Function& F ) override
    {
        if ( ClScorepPassVerbose >= 2 )
        {
            errs() << "Processing function " << F.getName() << "\n";
        }

        if ( !is_instrumentable( F ) )
        {
            /* function is not modified */
            return false;
        }


        Module* m = F.getParent();

        /* create types and function prototypes only once */
        if ( scorep_region_description_type == nullptr )
        {
            scorep_region_description_type = create_struct_type( m );
            std::tie( scorep_plugin_register_region_func,
                      scorep_plugin_enter_region_func,
                      scorep_plugin_exit_region_func,
                      personality_function ) = create_function_prototypes( m );
        }

        function_parsing_result_t parse_result = parse_function( F );

        Instruction& start_point = get_insert_begin( F );


        std::tuple< GlobalVariable*, GlobalVariable* > scorep_handle             = create_global_vars( F );
        GlobalVariable*                                scorep_region_handle      = std::get<0>( scorep_handle );
        GlobalVariable*                                scorep_region_description = std::get<1>( scorep_handle );

        insert_scorep_function_calls( F, start_point, parse_result.exit_points, scorep_region_handle, scorep_region_description );
        expection_handling( F, parse_result.call_instr, scorep_region_handle );

        return true;
    }

private:

    /**
     * Creates a tuple of a region handle and its descriptions.
     *
     * @param f         Function to process.
     *
     * @return Tuple of ( region_handle, region_description )
     */
    std::tuple<GlobalVariable*, GlobalVariable*>
    create_global_vars( Function& f )
    {
        Module*   m            = f.getParent();
        StringRef func_name    = f.getName();
        Type*     i32_ptr_type = Type::getInt32PtrTy( m->getContext() );
        Type*     i32_type     = Type::getInt32Ty( m->getContext() );
        Type*     i8_ptr_type  = Type::getInt8PtrTy( m->getContext() );

        auto metadata = get_meta_data( f );

        GlobalVariable* region_handle = new GlobalVariable( *m,
                                                            Type::getInt32Ty( m->getContext() ),
                                                            false,
                                                            GlobalValue::LinkageTypes::InternalLinkage,
                                                            ConstantInt::get( i32_type, 0 ),
                                                            func_name + ".scorep_region_handle" );
        region_handle->setAlignment( 4 );

        Constant* region_description_init = ConstantStruct::get(
            scorep_region_description_type,
            {
                ConstantExpr::getPointerCast( region_handle, i32_ptr_type ),
                ConstantExpr::getPointerCast( create_global_string( f,
                                                                    demangle( f.getName().str() ),
                                                                    "name" ),
                                              i8_ptr_type ),
                ConstantExpr::getPointerCast( create_global_string( f,
                                                                    f.getName().str(),
                                                                    "canonical_name" ),
                                              i8_ptr_type ),
                ConstantExpr::getPointerCast( create_global_string( f,
                                                                    std::get<2>( metadata ),
                                                                    "file" ),
                                              i8_ptr_type ),
                ConstantInt::get( i32_type,
                                  std::get<0>( metadata ) ),
                ConstantInt::get( i32_type,
                                  std::get<1>( metadata ) ),
                ConstantInt::get( i32_type, 0 )
            }
            );

        GlobalVariable* region_description = new GlobalVariable( *m, scorep_region_description_type,
                                                                 true,
                                                                 GlobalValue::LinkageTypes::InternalLinkage,
                                                                 region_description_init,
                                                                 func_name + ".scorep_region_descr",
                                                                 nullptr );
        region_description->setAlignment( 8 );

        return std::make_tuple( region_handle, region_description );
    }

    /**
     * Create prototypes for Score-P's register, enter, and exit function and inserts them to LLVM IR.
     *
     * @param m         Module in which context prototypes will be inserted.
     *
     * @return tuple of function prototypes (register, enter, exit, personality)
     */
    std::tuple<scorep_function_t*, scorep_function_t*, scorep_function_t*, personality_function_t*>
    create_function_prototypes( Module* m )
    {
        LLVMContext& ctx       = m->getContext();
        Type*        void_type = Type::getVoidTy( ctx );
        Type*        i32_type  = Type::getInt32Ty( ctx );

        PointerType* struct_ptr_type = PointerType::get( scorep_region_description_type, 0 );

        /** register linked Score-P functions */
        scorep_function_t* register_func = m->getOrInsertFunction( "scorep_plugin_register_region",
                                                                   void_type,
                                                                   struct_ptr_type,
                                                                   NULL );
        scorep_function_t* enter_func = m->getOrInsertFunction( "scorep_plugin_enter_region",
                                                                void_type,
                                                                i32_type,
                                                                NULL );
        scorep_function_t* exit_func = m->getOrInsertFunction( "scorep_plugin_exit_region",
                                                               void_type,
                                                               i32_type,
                                                               NULL );
        // EHPersonality::GNU_C personality should be always linked on C/C++ and is able to provide cleanup on exception handling
        // Actually we want to call getEHPersonalityName( EHPersonality::GNU_C ) that returns a StringRef to "__gcc_personality_v0"
        // However, getEHPersonalityName is only available in LLVM >= 4.0
        // Therefore, we use the string directly in the following function call
        personality_function_t* pers_func = m->getOrInsertFunction( "__gcc_personality_v0",
                                                                    i32_type,
                                                                    NULL );

        return std::make_tuple( register_func, enter_func, exit_func, pers_func );
    }


    /**
     * Inserts exception handling to every call instruction. Therefore insert a resume block if it does not already exist.
     * Also delete function attribute NoUnwind and set function's personality if it does not have one to perform
     * Score-P exit call when a function call receives an exception.
     *
     * @param f                     Function to process.
     * @param callInst              Vector of all call instructions which need to be processed.
     * @param scorepRegionHandle    Pointer to global variable of scorep_region_handle.
     */
    void
    expection_handling( Function&              f,
                        std::vector<CallInst*> callInst,
                        GlobalVariable*        scorepRegionHandle )
    {
        // exception handling is not necessary if there is no call and no invoke instruction, if there is any invoke
        // instruction there has to be a resume instruction
        if ( callInst.empty() )
        {
            return;
        }

        auto              tup             = create_unwind_resume_block( f );
        resume_block_t*   resume_block    = std::get<0>( tup );
        exception_slots_t exception_slots = std::get<1>( tup );

        BasicBlock* landing_pad = create_landingpad( f, resume_block, exception_slots );

        // change all call instructions to invoke instructions because we don't know if they could throw an exception
        // somewhere in the call stack
        for ( std::vector<CallInst*>::iterator it = callInst.begin(), itEnd = callInst.end(); it != itEnd; )
        {
            // modifying the iterator is not allowed
            CallInst* call = *it++;
            changeToInvokeAndSplitBasicBlock( call, landing_pad );
        }

        IRBuilder<> builder( f.getContext() );
        insert_exit_region( *resume_block->getFirstNonPHI(), builder, scorepRegionHandle );

        if ( f.hasFnAttribute( llvm::Attribute::NoUnwind ) )
        {
            f.removeFnAttr( llvm::Attribute::NoUnwind );
        }

        if ( !f.hasPersonalityFn() )
        {
            f.setPersonalityFn(
                ConstantExpr::getPointerCast( personality_function, Type::getInt8PtrTy( f.getContext(), 0 ) ) );
        }
    }


    /**
     * Inserts basic blocks with Score-P's function calls.
     *
     * @param f                         Function to process.
     * @param startPoint                Instruction where to insert register and enter call before.
     * @param exitPoints                Vector where to insert exit calls.
     * @param scorepRegionHandle        Reference to Score-P's region handle.
     * @param scorepRegionDescription   Reference to Score-P's region description.
     */
    void
    insert_scorep_function_calls( Function&                  f,
                                  Instruction&               startPoint,
                                  std::vector<Instruction*>& exitPoints,
                                  GlobalVariable*            scorepRegionHandle,
                                  GlobalVariable*            scorepRegionDescription )
    {
        IRBuilder<> builder( f.getContext() );

        insert_register_region( startPoint, builder, scorepRegionHandle, scorepRegionDescription );
        insert_enter_region( startPoint, builder, scorepRegionHandle );

        for ( Instruction* inst : exitPoints )
        {
            insert_exit_region( *inst, builder, scorepRegionHandle );
        }
    }


    /**
     * Insert basic block for function call of Score-P's register function surrounded by an if statement.
     *
     * @param insertPoint               Instruction where to insert the register call before.
     * @param builder                   IR builder.
     * @param scorepRegionHandle        Reference to Score-P's region handle.
     * @param scorepRegionDescription   Reference to Score-P's region description.
     */
    void
    insert_register_region( Instruction&    insertPoint,
                            IRBuilder<>&    builder,
                            GlobalVariable* scorepRegionHandle,
                            GlobalVariable* scorepRegionDescription )
    {
        builder.SetInsertPoint( &insertPoint );
        Value*          load                   = builder.CreateLoad( scorepRegionHandle );
        Value*          icmp                   = builder.CreateICmpEQ( load, builder.getInt32( 0 ) );
        TerminatorInst* register_plugin_branch = SplitBlockAndInsertIfThen( icmp, &insertPoint, false );
        builder.SetInsertPoint( register_plugin_branch );
        Value* gep = builder.CreateInBoundsGEP( scorepRegionDescription, { builder.getInt32( 0 ) } );
        builder.CreateCall( scorep_plugin_register_region_func, gep );
    }

    /**
     * Inserts basic block for function call of Score-P's enter function surrounded by an if statement.
     *
     * @param insertPoint           Instruction where to insert the 'function exit' call before.
     * @param builder               IR builder.
     * @param scorepRegionHandle    Reference to Score-P's region handle.
     */
    void
    insert_enter_region( Instruction&    insertPoint,
                         IRBuilder<>&    builder,
                         GlobalVariable* scorepRegionHandle )
    {
        builder.SetInsertPoint( &insertPoint );
        Value*          load                = builder.CreateLoad( scorepRegionHandle );
        Value*          icmp                = builder.CreateICmpNE( load, builder.getInt32( -1 ) );
        TerminatorInst* enter_plugin_branch = SplitBlockAndInsertIfThen( icmp, &insertPoint, false );
        builder.SetInsertPoint( enter_plugin_branch );
        builder.CreateCall( scorep_plugin_enter_region_func, load );
    }

    /**
     * Inserts basic block for function call of Score-P's exit function surrounded by an if statement.
     *
     * @param insertPoint           Instruction where to insert the 'function exit' call before.
     * @param builder               IR builder.
     * @param scorepRegionHandle    Reference to Score-P region handle.
     */
    void
    insert_exit_region( Instruction&    insertPoint,
                        IRBuilder<>&    builder,
                        GlobalVariable* scorepRegionHandle )
    {
        builder.SetInsertPoint( &insertPoint );
        Value*          load               = builder.CreateLoad( scorepRegionHandle );
        Value*          icmp               = builder.CreateICmpNE( load, builder.getInt32( -1 ) );
        TerminatorInst* exit_plugin_branch = SplitBlockAndInsertIfThen( icmp, &insertPoint, false );
        builder.SetInsertPoint( exit_plugin_branch );
        builder.CreateCall( scorep_plugin_exit_region_func, load );
    }


    /**
     * region description struct
     */
    StructType* scorep_region_description_type = nullptr;

    /** register function */
    scorep_function_t* scorep_plugin_register_region_func;

    /** enter function */
    scorep_function_t* scorep_plugin_exit_region_func;

    /** exit function */
    scorep_function_t* scorep_plugin_enter_region_func;

    /** personality function for exception handling */
    personality_function_t* personality_function;
};
}

char ScorepPass::ID = 0;


// Automatically enable the pass.
// http://adriansampson.net/blog/clangpass.html
static void
registerScorepPass( const PassManagerBuilder&,
                    legacy::PassManagerBase& PM )
{
    PM.add( new ScorepPass() );
}

// add pass after all optimizations
static RegisterStandardPasses
    RegisterMyPass( PassManagerBuilder::EP_OptimizerLast,
                    registerScorepPass );

// fallback for compiling with -O0. Pass will be executed after inlining
static RegisterStandardPasses
    RegisterMyPass0( PassManagerBuilder::EP_EnabledOnOptLevel0,
                     registerScorepPass );
