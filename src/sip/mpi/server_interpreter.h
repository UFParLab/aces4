/*
 * interpreter.h
 *
 * This is the SIAL interpreter.
 *
 * The global_interpreter is a static global pointer to the current interpreter instance.  This is needed
 * for super instructions written in fortran.
 *
 * The interpreter contains pointers to the SipTable and DataManager, which contain the static and dynamic data
 * for the current program, respectively.  A set of inline functions is given that simply provide convenient
 * access to the contents of these data structures.
 *
 *
 *
 *      Author: Beverly Sanders
 */

#ifndef SERVERINTERPRETER_H_
#define SERVERINTERPRETER_H_
#include <iostream>
#include <string>
#include <list>
#include <utility>
#include <set>
#include <stack>
#include <sstream>
#include "block_id.h"
#include "block_manager.h"
#include "contiguous_array_manager.h"
#include "data_manager.h"
#include "disk_backed_block_map.h"
#include "sialx_timer.h"
#include "config.h"
#include "sial_math.h"

#ifdef HAVE_MPI
#include "sial_ops_parallel.h"
#else
#include "sial_ops_sequential.h"
#endif //HAVE_MPI

class TestControllerParallel;
class TestController;

namespace sip {

class LoopManager;
class SialPrinter;
class Tracer;

enum server_array_state {
	outdated = 0,
	active = 1,
	tobeactive = 2
};

class ServerInterpreter {
public:

	ServerInterpreter(const SipTables&, DiskBackedBlockMap&);
	~ServerInterpreter();
    
	void handle_incoming_block_request(int array_id, int worker_rank, int line_number);
     
    void prefetch_block1();
    void free_block1();
    
	void handle_block_request(BlockId &block_id, int worker_rank, int line_number, 
                              int *loop_indices, int indices_num);
    
    void handle_put_block_request(BlockId &block_id, int worker_rank, int line_number);

    bool prefetch_block();
    bool write_block();
    void free_block();
    
    std::set<BlockId> &get_unused_blocks(){return unused_blocks;};
    std::set<BlockId> &get_dirty_blocks(){return dirty_blocks;};
    
    bool more_blocks_to_prefetch();
    bool more_blocks_to_free();
    bool more_blocks_to_prefetch_or_free();
    
    void remove_read_block(const BlockId &block_id);
    
    void print_workers_iterations();
    
private:

    DiskBackedBlockMap &block_map_;

    class ProgramBlock {
    public:
        int start_line_number;
        int end_line_number;
        
        int worker_number;
        
        std::set<int> get_arrays;
        std::set<int> update_arrays;
        std::set<int> put_arrays;
        std::map<int, int> array_distance;
        
        ProgramBlock() : start_line_number(0)
                       , end_line_number(0)
                       , worker_number(0) {};
        
        ~ProgramBlock() {};
    };

    std::vector<ProgramBlock> program_blocks;
    std::map<int, int> worker_position;
    
    std::set<int> prefetched_arrays;
    std::set<BlockId> prefetching_blocks;
    
    std::set<int> dead_arrays;
    std::set<BlockId> dead_blocks;
    
    int last_program_block;
    int first_program_block;
    
    void generate_program_blocks();
    
    int get_program_block_index(int line_number);
    
    int get_block_index(int line_number);
    
    void prefetch_arrays();
    void get_dead_arrays();
    
    void print_program_blocks();
    void print_op_table();
    
    enum LoopType {
        DO = 0,
        PARDO = 1,
    };
    
    class LoopBlock {
    public:
    
        class ArrayReference {
        public:
        
            int array_id;
            int rank;
            int index[6];
            
            ArrayReference() : array_id(0)
                             , rank(0)
            {
                for(int i = 0; i < 6; i ++) {
                    index[i] = 0;
                }
            }
        };
    
        LoopType loop_type;
        
        int start_line_number;
        int end_line_number;
        
        int worker_num[7];
        
        std::vector<int> indices;
        std::map<int, int> indices_index;
        
        std::vector<ArrayReference> get_arrays;
        std::vector<ArrayReference> update_arrays;
        
        LoopBlock() : loop_type(DO)
                    , start_line_number(0)
                    , end_line_number(0)
                    {
            for (int i = 0; i < 7; i ++) {
                worker_num[i] = 0;
            }
        };
        
        ~LoopBlock() {};
        
        void insert_get_array_reference(int array_id, int rank, const int *index) {
            ArrayReference array_reference;
            array_reference.array_id = array_id;
            array_reference.rank = rank;
            for (int i = 0; i < rank; i ++) {
                array_reference.index[i] = index[i];
            }
            get_arrays.push_back(array_reference);
        }
        
        void insert_update_array_reference(int array_id, int rank, const int *index) {
            ArrayReference array_reference;
            array_reference.array_id = array_id;
            array_reference.rank = rank;
            for (int i = 0; i < rank; i ++) {
                array_reference.index[i] = index[i];
            }
            update_arrays.push_back(array_reference);
        }
    };
    
    class WorkerIteration {
    public:
        int loop_index;
        int indices_num;
        int indices[6];
        
        std::set<BlockId> blocks;
        std::set<BlockId> prefetch_blocks;
        
        WorkerIteration() : loop_index(0)
                          , indices_num(0) {
            for (int i = 0; i < 6; i ++) {
                indices[i] = 0;
            }
        }
    };

    void generate_loop_blocks();
    
    void add_new_worker_iteration(int loop_index, LoopBlock &loop_block, int indices_value[7],
                                  std::list<WorkerIteration> &worker_iterations);

    void remove_worker_iteration(std::list<WorkerIteration> &worker_iterations);
    
    std::map<int, std::list<WorkerIteration> > workers_iteration;
    std::map<BlockId, int> block_reference;
    
    std::set<BlockId> unused_blocks;
    std::set<BlockId> dirty_blocks;
    
    std::set<BlockId> pending_dirty_blocks;
    
    std::vector<LoopBlock> loop_blocks;

    void print_loop_blocks();

	//static data
	const SipTables& sip_tables_;

	const OpTable &op_table_;  //owned by sipTables_, pointer copied for convenience

	friend class ::TestControllerParallel;
	friend class ::TestController;

	DISALLOW_COPY_AND_ASSIGN(ServerInterpreter);
};
/* class Interpreter */

} /* namespace sip */

#endif /* INTERPRETER_H_ */