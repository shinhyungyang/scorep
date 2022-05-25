/*
 * This file is part of the Score-P software (http://www.score-p.org)
 *
 * Copyright (c) 2012-2014, 2016,
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
 * @brief  Implementation of a LLVM pass in order to count basic blocks and
 *         instructions.
 *
 */

#include "llvm/Pass.h"
#include "llvm/Transforms/IPO/PassManagerBuilder.h" /* PassManagerBuilder  */
#include "llvm/Support/raw_ostream.h"               /* errs()              */
#include "llvm/Analysis/LoopInfo.h"                 /* LoopInfoWrapperPass */
#include "llvm/IR/DebugInfoMetadata.h"              /* DISubprogram        */
#include "llvm/IR/IRBuilder.h"                      /* IRBuilder           */
#include "llvm/IR/LegacyPassManager.h"              /* PM                  */
#include "llvm/IR/Instruction.h"                    /* Instruction         */
#include "llvm/IR/Function.h"


using namespace llvm;

/************************** GLOBAL VARIABLES  *********************************/

int bid = 0;

/******************************** DECLARATIONS ********************************/

int32_t getLID(Instruction* BI);

/******************************** DEFINITIONS *********************************/

namespace {
    struct BasicBlockPass : public FunctionPass {
        static char ID;
        BasicBlockPass() : FunctionPass(ID) {}

        void getAnalysisUsage(AnalysisUsage &AU) const override {
            AU.addRequired<LoopInfoWrapperPass>();
        }

        virtual bool runOnFunction(Function &F) {
            Type         *Void;
            IntegerType  *Int32;
            LLVMContext  &Ctx     = F.getContext();
            DISubprogram *dis     = F.getSubprogram();

            Void    = Type::getVoidTy(Ctx);
            Int32   = Type::getInt32Ty(Ctx);

            auto bbFunc    = F.getParent()->getOrInsertFunction(
                                        "bbCounter", Void, Int32,Int32,Int32,Int32);

            auto printFunc = F.getParent()->getOrInsertFunction(
                                        "printOut", Void);

            int fun_ln = dis->getScopeLine(); // Get the function line number

            for (Function::iterator BB=F.begin(), BE=F.end(); BB!=BE; ++BB) {
                for (auto& I : *BB) {
                    BasicBlock *tmpBB = &*BB;
                    StringRef  fn     = F.getName();

                    if (fn.equals("main")) {
                        if (isa<ReturnInst>(I)) {
                            try {
                                IRBuilder<> builder(&*BB->rbegin());
                                 builder.CreateCall(printFunc);
                            }
                            catch(std::exception &e) {
                                errs() << e.what() << "\n" ;
                            }
                        }
                    }
                }
            }

            for (Function::iterator BB=F.begin(), BE=F.end(); BB!=BE; ++BB) {
                int inscnt            = 0;
                int bbLine_initial    = 0;
                int bbLine_final      = 0;
                int bbLine_final_temp = 0;
                int bbLine_temp       = 0;

                BasicBlock::iterator I = (*BB).begin();
                BasicBlock *tmpBB      = &*BB;

                inscnt = (*tmpBB).sizeWithoutDebug();

                for (BasicBlock::iterator I = (*BB).begin(); I != (*BB).end(); I++) {
                    Instruction *tmpI = &*I;
                    bbLine_temp = getLID(&*tmpI);

                    if( (bbLine_initial == 0) && (bbLine_temp != 0) ) {
                        bbLine_initial = bbLine_temp;

                        if(bbLine_temp == fun_ln) {
                            bbLine_initial = 0;
                        }
                    }
                    if( (bbLine_initial != 0) && (bbLine_temp != 0) ) {
                      bbLine_final      = bbLine_temp;
                      bbLine_final_temp = bbLine_temp;
                    }
                    if( (bbLine_initial != 0) && (bbLine_final != 0) && (bbLine_temp == 0) ) {
                      bbLine_final++;
                    }
                }

                bid++; // ID of the basic block
                errs() << "BB id: " << bid << " Initial line: " << bbLine_initial
                << " Final line: " << bbLine_final << " Inst count: " << inscnt << "\n";
                IRBuilder<> IRB(&*BB->begin());
                ArrayRef<Value *> arguments( {ConstantInt::get(Int32, bid),
                                              ConstantInt::get(Int32, inscnt),
                                              ConstantInt::get(Int32, bbLine_initial),
                                              ConstantInt::get(Int32, bbLine_final)} );
                IRB.CreateCall(bbFunc, arguments); // Instrumentation function
            }
            return false;
        } /* runOnFunction */
    };    /* struct BasicBlockPass */
}

char BasicBlockPass::ID = 0;
// Automatically enable the pass.

static void registerSkeletonPass(const PassManagerBuilder &,
                         legacy::PassManagerBase &PM) {
  PM.add(new BasicBlockPass());
}

static RegisterStandardPasses
  RegisterMyPass(PassManagerBuilder::EP_EarlyAsPossible,
                 registerSkeletonPass);

/******************************************************************************/

int32_t getLID(Instruction* BI) {
    int32_t lno = 0;
    const DebugLoc &location = BI->getDebugLoc();

    if (location) {
        lno = BI->getDebugLoc().getLine();
    }

    return lno;
}

/******************************************************************************/