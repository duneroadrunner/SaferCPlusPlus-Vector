SaferCPlusPlus-Vector
=====================
Jan 2015

Mainly, a safe implementation of std::vector<>. I.e. bounds checked and "invalid iterator dereference" checked. Also, a couple of other highly compatible vectors that address the issue of unecessary iterator invalidation upon insert, erase or reallocation. Also, replacements for the native "int", "size_t" and "bool" types that have default initialization values and automatically address the "signed-unsigned mismatch" issues.

Tested with msvc2013, msvc2010 and g++4.8 (as of Jan 2015).

See the file msetl/msetl_blurb.rtf or msetl/msetl_blurb.html for more info.
