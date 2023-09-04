//
// License: MIT
//
#include "sample.h"

void TopFunction(int) {}
void TopFunction(std::string &) {}
void HiddenTopFunction(int) {}
double earth::creatures::Home::Method(std::string &, int) { return 0; }
int earth::creatures::SweetHome::Method(int i) { return i; }
void earth::creatures::SweetHome::Method(std::string &) {}
void earth::creatures::SweetHome::StaticMethod(int) {}
void earth::creatures::SweetHome::StaticMethod(std::string &) {}
void earth::creatures::SweetHome::PrivateMethod(int) {}
void earth::creatures::SweetHome::PrivateStaticMethod(int) {}
void earth::creatures::SweetHome::HiddenMethod(int) {}
int earth::creatures::NSFunction(const std::string &str) { return 0; }
