#ifndef SILC_WRAPGEN_HELP_H_
#define SILC_WRAPGEN_HELP_H_

#include <iostream>
using std::cout;
using std::endl;

namespace SILC
{
namespace Wrapgen
{
void
help
(
    string name
)
{
    cout << "'" << name << "' generates EPILOG measurement wrappers for MPI function\n" << endl;

    cout << "USAGE :" << name << " -p <prototypes.xml> [OPTIONS] <templatefile> [<template2> ... ]\n" << endl;
    cout << "If more than one template file are specified, the output of all templates" << endl;
    cout << "is appended.\n" << endl;
    cout << "TEMPLATE FILE TYPES:" << endl;
    cout << ".txt   are interpreted as text files and the content is copied." << endl;
    cout << ".w     are interpreted as wrapper templates. All selected functions" << endl;
    cout << "       from the prototypes are processed using the given wrapper template." << endl;
    cout << ".tmpl  A template file which contains basic code and template configurations" << endl;
    cout << "       via #pragma wrapgen directives. When using this format, the -r" << endl;
    cout << "       option takes no effect for this file.\n" << endl;
    cout << "OPTIONS:" << endl;
    cout << "-b   disables the banner output before generating new code\n" << endl;
    cout << "-r   (restrict option) can specify the output depending on the specifier" << endl;
    cout << "     g   specifies the group : '" << name << " -rgp2p' (for Peer to Peer)" << endl;
    cout << "     i   specifies the ID of a function: '" << name << " -ri92' (Function with ID 92)" << endl;
    cout << "     n   specifies a string in the functionname: '" << name << " -rncast'" << endl;
    cout << "         (all function including the string 'cast')" << endl;
    cout << "     t   specifies the returntype of the MPI function: '" << name << " -rtint'" << endl;
    cout << "         (all functions return an int)" << endl;
    cout << "     v   specifies the version: '" << name << " -rv1' (for MPI Version 1)\n" << endl;
    cout << "     A combination of -r options is possible using a '+' between the options." << endl;
    cout << "     Example: " << name << " -p proto.xml -rntype+v1+gp2p test.w" << endl;
    cout << "     Process all functions in proto.xml which contain the string 'type'," << endl;
    cout << "     and belong to MPI Version 1, and belong to group" << endl;
    cout << "     'peer-to peer communication'.\n" << endl;
    cout << "-h   Prints this help.\n"  << endl;
    exit( 0 );
}
}   // namespace Wrapgen
}   // namespace SILC


#endif /* SILC_WRAPGEN_HELP_H_ */
