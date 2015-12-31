#ifndef _TYPEDEF_
#define _TYPEDEF_

#include "tbl_cup_succ.h"

typedef void (*OnTokenFindFunc)(void* ctx, int index, const char* token);
typedef void (*OnTokenOverFunc)(void* ctx, int index, const char* token);

typedef int (*OnLineFindFunc)(void* ctx, int index, const char* line);
typedef int (*OnLineOverFunc)(void* ctx, int index, const char* line);

typedef enum _FIELD_TYPE {
	STRING,
	FLOAT,
	NONE
}FIELD_TYPE;

typedef struct _StatementField
{
	size_t index;
	FIELD_TYPE type;
	void* addr;
}StatementField;

typedef struct _X
{
	const char* name;
	size_t index;
	FIELD_TYPE type;
	void* addr;
}X;

typedef struct _TokenCtx
{
	TCupSucc* cup_succ;
	StatementField* field;
}TokenCtx;

typedef struct _RelocateCupSucc
{
	const char* name;
	size_t	addr;
}RelocateCupSucc;

int fileline_foreach(const char* file_name, OnLineFindFunc on_line_find, OnLineOverFunc on_line_over, void* ctx);
int delims_token_parse(const char delim, OnTokenFindFunc on_token_find, OnTokenOverFunc on_token_over, void* ctx, void* data);
void on_token_find(void* ctx, int index, const char* token);


#endif
