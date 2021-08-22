#ifndef ALISP_FAIL_H
#define ALISP_FAIL_H

#include <stdio.h>
#include <stdlib.h>

#define a_fail(spec, ...)				\
  fprintf(stderr, "Error in %s, line %d:\n" spec,	\
	  __FILE__, __LINE__, ##__VA_ARGS__);		\
  abort();					  

#endif
