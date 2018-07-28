#include "base/stats.h"
namespace crown{

int Stats::max_sym_path_len;
int Stats::min_sym_path_len;
int Stats::max_con_path_len;
int Stats::min_con_path_len;
double Stats::avg_sym_path_len;
double Stats::avg_con_path_len;

void Stats::UpdateSymbolicPathLength(int len){
	static int n = 0;
	n++;
	avg_sym_path_len += ((double)len - avg_sym_path_len) / (double)n;
	if (len < min_sym_path_len || min_sym_path_len == 0) min_sym_path_len = len;
	else if (len > max_sym_path_len) max_sym_path_len = len;
}

void Stats::UpdateConcretePathLength(int len){
	static int n = 0;
	n++;
	avg_con_path_len += ((double)len - avg_con_path_len) / (double)n;
	if (len < min_con_path_len || min_con_path_len == 0) min_con_path_len = len;
	else if (len > max_con_path_len) max_con_path_len = len;
}


}
