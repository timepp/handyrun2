#ifndef TP_DEF_H_INCLUDED
#define TP_DEF_H_INCLUDED

#define CONCAT_INNER(a,b) a##b
#define CONCAT(a,b) CONCAT_INNER(a,b)
#define LINE_NAME(prefix) CONCAT(prefix,__LINE__)

#endif