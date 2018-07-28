#ifndef BASE_STATS_H_
#define BASE_STATS_H_
namespace crown{
class Stats{
public:
	static void UpdateSymbolicPathLength(int);
	static void UpdateConcretePathLength(int);
	static double GetAverageSymbolicPathLength(){ return avg_sym_path_len; }
	static double GetAverageConcretePathLength(){ return avg_con_path_len; }
	static int GetMaxSymbolicPathLength(){ return max_sym_path_len; }
	static int GetMaxConcretePathLength(){ return max_con_path_len; }
	static int GetMinSymbolicPathLength(){ return min_sym_path_len; }
	static int GetMinConcretePathLength(){ return min_con_path_len; }
private:
	static int max_sym_path_len;
	static int min_sym_path_len;
	static int max_con_path_len;
	static int min_con_path_len;
	static double avg_sym_path_len;
	static double avg_con_path_len;
};

}
#endif
