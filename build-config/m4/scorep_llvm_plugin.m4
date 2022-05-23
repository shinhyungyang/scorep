dnl -*- mode: autoconf -*-

dnl
dnl This file is part of the Score-P software (http://www.score-p.org)
dnl
dnl Copyright (c) 2017,
dnl Technische Universitaet Dresden, Germany
dnl
dnl This software may be modified and distributed under the terms of
dnl a BSD-style license.  See the COPYING file in the package base
dnl directory for details.
dnl

AC_DEFUN([_SCOREP_LLVM_PLUGIN_CHECK], [
AC_REQUIRE([LT_OUTPUT])
AC_REQUIRE([_SCOREP_LIBBFD])

# Get CFLAGS required to compile with LLVM/Clang
scorep_llvm_cflags="-std=c++11 $(llvm-config --cflags) -fno-rtti"

AC_LANG_PUSH($1)

save_CPPFLAGS=$CPPFLAGS
CPPFLAGS="$CPPFLAGS ${scorep_llvm_cflags}"

# header checks
scorep_llvm_have_plugin_headers=yes
AC_CHECK_HEADERS([llvm/Pass.h llvm/IR/Function.h llvm/Support/raw_ostream.h llvm/IR/LegacyPassManager.h llvm/IR/InstrTypes.h llvm/Transforms/IPO/PassManagerBuilder.h llvm/IR/IRBuilder.h llvm/IR/BasicBlock.h llvm/Transforms/Utils/BasicBlockUtils.h llvm/IR/Module.h llvm/Transforms/Utils/Local.h llvm/Analysis/EHPersonalities.h llvm/IR/DebugInfo.h],
                 [],
                 [scorep_llvm_have_plugin_headers=no])

AC_MSG_CHECKING([for LLVM $1 plug-in headers])
AC_MSG_RESULT([${scorep_llvm_have_plugin_headers}])

# header check for LLVM's internal demangle
scorep_have_llvm_demangle="no"
AC_CHECK_HEADER([llvm/Demangle/Demangle.h],
    [scorep_have_llvm_demangle=yes],
    [scorep_have_llvm_demangle=no])

AS_IF([test "x${scorep_llvm_have_plugin_headers}" = "xyes"],
      [# minimal Clang plug-in
       AC_LANG_CONFTEST([AC_LANG_SOURCE([[
#include <llvm/Pass.h>
#include <llvm/IR/Function.h>
#include <llvm/Support/raw_ostream.h>
#include <llvm/IR/LegacyPassManager.h>
#include <llvm/IR/InstrTypes.h>
#include <llvm/Transforms/IPO/PassManagerBuilder.h>
#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/BasicBlock.h>
#include <llvm/IR/Module.h>

using namespace llvm;

namespace
{
    struct InstrumentPass : public FunctionPass
    {
        static char ID;

        InstrumentPass() : FunctionPass( ID ) {}

        virtual bool runOnFunction( Function &F )
        {
            return false;
        }
    };
}

char InstrumentPass::ID = 0;

static void
registerInstrumentPass( const PassManagerBuilder &,
                        legacy::PassManagerBase &PM )
{
    PM.add( new InstrumentPass() );
}

static RegisterStandardPasses
RegisterMyPass( PassManagerBuilder::EP_EarlyAsPossible,
                registerInstrumentPass );
]])])

    # build plug-in with libtool to get an shared object
    # -rpath is needed, else libtool will only build an convenient library
    AC_MSG_CHECKING([to build a $1 plug-in])
    plugin_compile='$SHELL ./libtool --mode=compile --tag=_AC_CC [$]_AC_CC $CPPFLAGS [$]_AC_LANG_PREFIX[FLAGS] -c -o conftest.lo conftest.$ac_ext >&AS_MESSAGE_LOG_FD'
    plugin_link='$SHELL ./libtool --mode=link --tag=_AC_CC [$]_AC_CC [$]_AC_LANG_PREFIX[FLAGS] -module -avoid-version $LDFLAGS -rpath $PWD/lib -o confmodule.la conftest.lo >&AS_MESSAGE_LOG_FD'
    plugin_mkdir='$MKDIR_P lib >&AS_MESSAGE_LOG_FD'
    plugin_install='$SHELL ./libtool --mode=install $INSTALL confmodule.la $PWD/lib/confmodule.la >&AS_MESSAGE_LOG_FD'
    plugin_clean='$SHELL ./libtool --mode=clean $RM conftest.lo confmodule.la >&AS_MESSAGE_LOG_FD'
    AS_IF([_AC_DO_VAR([plugin_compile]) &&
           _AC_DO_VAR([plugin_link]) &&
           _AC_DO_VAR([plugin_mkdir]) &&
           _AC_DO_VAR([plugin_install])],
          [AC_MSG_RESULT([yes])

           # now try to use this plug-in in a compile test, always use the C
           # compiler, $1 is about in what language the compiler is written,
           # not what language the compiler should compile
           AC_LANG_PUSH([C])

           save_target_CC=$CC
           save_target_CPPFLAGS=$CPPFLAGS
           save_target_CFLAGS=$CFLAGS
           CC=$INSTRUMENTATION_PLUGIN_TARGET_CC
           CPPFLAGS=$INSTRUMENTATION_PLUGIN_TARGET_CPPFLAGS
           CFLAGS="$INSTRUMENTATION_PLUGIN_TARGET_CFLAGS -Xclang -load -Xclang $PWD/lib/confmodule.so"

           AC_MSG_CHECKING([to load a $1 plug-in])
           AC_COMPILE_IFELSE([AC_LANG_PROGRAM([], [])],
                             [scorep_llvm_have_working_plugin=yes
                              AC_MSG_RESULT([yes])],
                             [scorep_llvm_plugin_support_reason="no, failed to load plug-in"
                              AC_MSG_RESULT([no])])

           CC=$save_target_CC
           CPPFLAGS=$save_target_CPPFLAGS
           CFLAGS=$save_target_CFLAGS

           AS_UNSET([save_target_CC])
           AS_UNSET([save_target_CPPFLAGS])
           AS_UNSET([save_target_CFLAGS])

           AC_LANG_POP([C])],
          [_AC_MSG_LOG_CONFTEST
           AC_MSG_RESULT([no])])],
      [scorep_llvm_have_working_plugin=no
       scorep_llvm_plugin_support_reason="no, missing plug-in headers, please install"])

CPPFLAGS=$save_CPPFLAGS
AS_UNSET([save_CPPFLAGS])

AC_LANG_POP($1)

AC_MSG_CHECKING([for working LLVM $1 plug-in support])
AS_IF([test "x${scorep_llvm_have_working_plugin}" = "xyes"],
    [AC_MSG_RESULT([yes])
    scorep_llvm_plugin_support_reason="yes, using the $1 compiler"
    $2
    :],
    [AC_MSG_RESULT([no])
    $3
    :])

AS_UNSET([scorep_llvm_have_plugin_headers])
AS_UNSET([scorep_llvm_have_working_plugin])
])

# _SCOREP_LLVM_PLUGIN_TARGET_VERSION
# ---------------------------------
# Provides the LLVM version for the target LLVM as a number
# version = major * 1000 + minor
AC_DEFUN([_SCOREP_LLVM_PLUGIN_TARGET_VERSION], [

# 'llvm-config --version' returns major.minor.bugfix
scorep_llvm_plugin_target_version_dump="$(llvm-config --version)"

scorep_llvm_plugin_target_version=0
AS_IF([test "x${scorep_llvm_plugin_target_version_dump}" != "x"],
      [scorep_llvm_plugin_target_version_major=${scorep_llvm_plugin_target_version_dump%%.*}
       scorep_llvm_plugin_target_version_minor=${scorep_llvm_plugin_target_version_dump#*.}
       scorep_llvm_plugin_target_version_minor=${scorep_llvm_plugin_target_version_minor%%.*}
       scorep_llvm_plugin_target_version=$(expr ${scorep_llvm_plugin_target_version_major} "*" 1000 + ${scorep_llvm_plugin_target_version_minor})])
AC_DEFINE_UNQUOTED([SCOREP_LLVM_PLUGIN_TARGET_VERSION],
    [${scorep_llvm_plugin_target_version}],
    [The LLVM version for what we build the plug-in.])
])

# SCOREP_LLVM_PLUGIN
# -----------------
# Performs checks whether the LLVM/Clang compiler has plug-in support.
AC_DEFUN([SCOREP_LLVM_PLUGIN], [
AC_REQUIRE([_SCOREP_LLVM_PLUGIN_TARGET_VERSION])dnl

scorep_llvm_plugin_support="no"

# version checks
AS_IF([test ${scorep_llvm_plugin_target_version} -lt 20000],
      [scorep_llvm_plugin_support_reason="no, LLVM ${scorep_llvm_plugin_target_version_dump} is too old"],
      [_SCOREP_LLVM_PLUGIN_CHECK([C++],
                                 [AFS_AM_CONDITIONAL([LLVM_COMPILED_WITH_CXX], [true], [false])
                                  scorep_llvm_plugin_support="yes"])])

AFS_AM_CONDITIONAL([HAVE_LLVM_PLUGIN_SUPPORT], [test "x${scorep_llvm_plugin_support}" = "xyes"], [false])

AFS_AM_CONDITIONAL([HAVE_LLVM_DEMANGLE], [test "x${scorep_have_llvm_demangle}" = "xyes"], [false])

AFS_SUMMARY([LLVM plug-in support], [${scorep_llvm_plugin_support_reason}])

AM_COND_IF([HAVE_LLVM_PLUGIN_SUPPORT],
           [AC_SUBST([SCOREP_LLVM_PLUGIN_CPPFLAGS], ["${scorep_llvm_cflags}"])])

AS_UNSET([scorep_have_llvm_demangle])
AS_UNSET([scorep_llvm_cflags])
AS_UNSET([scorep_llvm_plugin_support_reason])
])
