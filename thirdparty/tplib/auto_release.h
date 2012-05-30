#ifndef TP_AUTO_RELEASE_H_INCLUDED
#define TP_AUTO_RELEASE_H_INCLUDED

#include "defs.h"

/**
 auto execute statements when leaving current scope

 @example 
   ON_LEAVE(printf("block exit"));
   ...
   FILE* fp = fopen("a.txt");
   ON_LEAVE_1(fclose(fp), FILE*, fp);

 @ref
   - scope guard in D language: http://www.digitalmars.com/d/2.0/statement.html#ScopeGuardStatement
   - boost scope exit: http://www.google.com/#q=boost+scope+exit
 @note
   It will become more elegant in C++0X
 */


#define ON_LEAVE(statement) \
	struct LINE_NAME(ols_) { \
		~LINE_NAME(ols_)() { statement; } \
	} LINE_NAME(olv_);

#define ON_LEAVE_1(statement, type, var) \
	struct LINE_NAME(ols_) { \
		type var; \
		LINE_NAME(ols_)(type v): var(v) {} \
		~LINE_NAME(ols_)() { statement; } \
	} LINE_NAME(olv_)(var);

#define ON_LEAVE_2(statement, type1, var1, type2, var2) \
	struct LINE_NAME(ols_) { \
		type1 var1; type2 var2; \
		LINE_NAME(ols_)(type1 v1, type2 v2): var1(v1), var2(v2) {} \
		~LINE_NAME(ols_)() { statement; } \
	} LINE_NAME(olv_)(var1, var2);

#define ON_LEAVE_3(statement, type1, var1, type2, var2, type3, var3) \
	struct LINE_NAME(ols_) { \
		type1 var1; type2 var2; type3 var3; \
		LINE_NAME(ols_)(type1 v1, type2 v2, type3 v3): var1(v1), var2(v2), var3(v3) {} \
		~LINE_NAME(ols_)() { statement; } \
	} LINE_NAME(olv_)(var1, var2, var3);

#define ON_LEAVE_4(statement, type1, var1, type2, var2, type3, var3, type4, var4) \
	struct LINE_NAME(ols_) { \
		type1 var1; type2 var2; type3 var3; type4 var4; \
		LINE_NAME(ols_)(type1 v1, type2 v2, type3 v3, type4 v4): var1(v1), var2(v2), var3(v3), var4(v4) {} \
		~LINE_NAME(ols_)() { statement; } \
	} LINE_NAME(olv_)(var1, var2, var3, var4);


#endif