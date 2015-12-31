#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "framework.h"

#define OFFSET(type,field) (int)&(((type*)0)->field)
#define ADDR(base, offset) (int)base + (int)offset

RelocateCupSucc cup_succ[] = 
{
	{"id", OFFSET(TCupSucc, id)},
	{"stlm_date", OFFSET(TCupSucc, stlm_date)},
	{"tx_date", OFFSET(TCupSucc, tx_date)},
	{"tx_time", OFFSET(TCupSucc, tx_time)},
	{"trace_no", OFFSET(TCupSucc, trace_no)},
	{"cup_key", OFFSET(TCupSucc, cup_key)},
	{"result_flag", OFFSET(TCupSucc, result_flag)},
	{"cups_no", OFFSET(TCupSucc, cups_no)},
	{"term_no", OFFSET(TCupSucc, term_no)},
	{"ac_no", OFFSET(TCupSucc, ac_no)},
	{"ac_bank_no", OFFSET(TCupSucc, ac_bank_no)},
	{"tx_amt", OFFSET(TCupSucc, tx_amt)},
	{"cup_fee", OFFSET(TCupSucc, cup_fee)},
	{"tx_code", OFFSET(TCupSucc, tx_code)},
	{"sys_ref_no", OFFSET(TCupSucc, sys_ref_no)},
	{"brf", OFFSET(TCupSucc, brf)},
	{"fill", OFFSET(TCupSucc, fill)},
	{"rec_crt_ts", OFFSET(TCupSucc, rec_crt_ts)},
	{"rec_crt_ts", OFFSET(TCupSucc, rec_crt_ts)},
	{"res", OFFSET(TCupSucc, res)},
	{"cups_nm", OFFSET(TCupSucc, cups_nm)},
	{"ac_bank_name", OFFSET(TCupSucc, ac_bank_name)}

};

void relocate_cupsucc(TCupSucc* base, X* x)
{
	int size = 	sizeof(cup_succ) / sizeof(cup_succ[0]);
	int i = 0;
	int j = 0;

	for(i = 0; i < size; i++) {
		cup_succ[i].addr = ADDR(base, cup_succ[i].addr);
	}

	i = 0;
	while(strlen(x[i].name) != 0) {
		for(j = 0; j < size; j++) {
			if (strcmp(x[i].name, cup_succ[j].name) == 0) {
				x[i].addr = cup_succ[j].addr;
				break;
			}
		}	
		i += 1;
	}

	return;
}

void on_token_find(void* ctx, int index, const char* token)
{
	StatementField* fields = ((TokenCtx*)ctx)->field;

	while(fields->index != -1)
	{
		if(fields->index == index)
		{
			if(fields->type == STRING)
			{
				strcpy(fields->addr, token);
			}
			else if (fields->type == FLOAT)
			{
				*((double*)(fields->addr)) = atof(token);
			}
			else 
			{
				return;
			}

			break;
		}

		fields += 1;
	}

	return;
}

int fileline_foreach(const char* file_name, OnLineFindFunc on_line_find, OnLineOverFunc on_line_over, void* ctx)
{
	size_t cursor = 0;

	FILE* fp = fopen(file_name, "r");
	if (fp == NULL)
	{
		return -1;
	}

	char buf[1024] = { 0 };
	int ret = 0;
	while(fgets(buf, sizeof(buf), fp))
	{
		ret = on_line_find(ctx, ++cursor, buf);
		if (ret < 0)
		{
			goto EXIT;
		}

		memset(buf, 0, sizeof(buf));
	}

	on_line_over(ctx, -1, NULL);

EXIT:
	fclose(fp);

	return 0;
}

int delims_token_parse(const char delim, OnTokenFindFunc on_token_find, OnTokenOverFunc on_token_over, void* ctx, void* data)
{
	size_t i,j,cursor,str_len;
	char token[255] = { 0 };

	i = j = cursor = 0;
	char* buf = (char*)data;
	str_len = strlen(buf);

	while(j < str_len)
	{
		for(; j < str_len; j++)
		{
			if(buf[j] == delim)
			{
				memcpy(token, buf + i, j - i);
				on_token_find(ctx, ++cursor, token);
				memset(token, 0, sizeof(token));
				i = j = j + 1;
				break;
			}
		}
	}

	if(i <= str_len)
	{
		memcpy(token, buf + i, str_len - i);
		on_token_find(ctx, ++cursor, token);
	}

	on_token_over(ctx, -1, NULL);

	return 0;
}
 
