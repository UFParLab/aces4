/*
 * Interpreter.cpp
 *
 *  Created on: Aug 5, 2013
 *      Author: Beverly Sanders
 */

#include "server_interpreter.h"
#include <vector>
#include <assert.h>
//#include <sstream>
#include <string>
#include <algorithm>
#include <iomanip>

#include "aces_defs.h"
#include "loop_manager.h"
#include "special_instructions.h"
#include "block.h"
#include "tensor_ops_c_prototypes.h"
#include "worker_persistent_array_manager.h"
#include "sial_printer.h"
#include "tracer.h"
#include "sial_math.h"
#include "config.h"

#include "TAU.h"

// For CUDA Super Instructions
#ifdef HAVE_CUDA
#include "gpu_super_instructions.h"
#endif

namespace sip {

ServerInterpreter::ServerInterpreter(const SipTables& sipTables, DiskBackedBlockMap& block_map)
                   : sip_tables_(sipTables)
                   , op_table_(sipTables.op_table_)
                   , last_program_block(0)
                   , first_program_block(-1)
                   , block_map_(block_map) {
    //print_op_table();
    //generate_loop_blocks();
    
    generate_program_blocks();
    
    if (program_blocks.size() > 0) {
        block_map_.update_array_distance(program_blocks[0].array_distance);
    }
    
    //print_op_table();
    //print_loop_blocks();
    //sip_abort();
}

ServerInterpreter::~ServerInterpreter() {

}

void ServerInterpreter::generate_program_blocks() {
    ProgramBlock program_block;
    int start_number = 0;
    int basic_reference = 0;
    
    std::set<int> arrays;
    
    program_blocks.clear();
    
    for (int i = 0; i < op_table_.size(); i ++) {
        
        if (op_table_.opcode(i) == get_op) {
            int array_id = op_table_.arg0(i);
            program_block.get_arrays.insert(array_id);
            arrays.insert(array_id);
        } else if (op_table_.opcode(i) == put_accumulate_op) {
            int array_id = op_table_.arg1(i);
            program_block.update_arrays.insert(array_id);
            arrays.insert(array_id);
        } else if (op_table_.opcode(i) == put_replace_op) {
            int array_id = op_table_.arg1(i);
            program_block.put_arrays.insert(array_id);
            arrays.insert(array_id);
        }
        
        if (op_table_.opcode(i) == endpardo_op) {
            program_block.start_line_number = op_table_.line_number(start_number);
            program_block.end_line_number   = op_table_.line_number(i);
            
            program_blocks.push_back(program_block);
            program_block = ProgramBlock();
            
            start_number = i + 1;
        }
    }
    
    if (program_blocks.size() > 0) {
        program_blocks[program_blocks.size()-1].end_line_number = op_table_.line_number(op_table_.size()-1);
    }
    
    std::map<int, int> array_distance;
    
    for (std::set<int>::iterator iter = arrays.begin(); iter != arrays.end(); iter ++) {
        array_distance[*iter] = -1;
    }
    
    for (int i = program_blocks.size()-1; i >= 0; i --) {
        for (std::map<int, int>::iterator iter = array_distance.begin(); iter != array_distance.end(); iter ++) {
            if (iter->second != -1) {
                iter->second ++;
            }
        }
        
        for (std::set<int>::iterator iter = program_blocks[i].get_arrays.begin();
             iter != program_blocks[i].get_arrays.end();
             iter ++) {
            array_distance[*iter] = 0;
        }
        
        for (std::set<int>::iterator iter = program_blocks[i].update_arrays.begin();
             iter != program_blocks[i].update_arrays.end();
             iter ++) {
            array_distance[*iter] = 0;
        }
        
        program_blocks[i].array_distance = array_distance;
    }
}

int ServerInterpreter::get_program_block_index(int line_number) {
    int l = 0, h = program_blocks.size() - 1;
    
    while (l < h) {
        int mid = (l + h) / 2;
    
        if (program_blocks[mid].start_line_number <= line_number
            && program_blocks[mid].end_line_number >= line_number) {
            return mid;
        }
        else if (program_blocks[mid].start_line_number > line_number) {
            h = mid - 1;
        }
        else {
            l = mid + 1;
        }
    }
    
    return l;
}

void ServerInterpreter::handle_incoming_block_request(int array_id, int worker_rank, int line_number) {
    int old_block = -1, new_block = -1;

    //std::cout << "Handle block request from worker: " << worker_rank << ", line number: " << line_number << std::endl;
    
    if (worker_position.count(worker_rank)) {
        old_block = get_program_block_index(worker_position[worker_rank]);
    }
    
    worker_position[worker_rank] = line_number;
    new_block = get_program_block_index(worker_position[worker_rank]);
    
    if (old_block != -1) {
        program_blocks[old_block].worker_number --;
    }
    
    program_blocks[new_block].worker_number ++;
    
    if (new_block > first_program_block) {
        first_program_block = new_block;
        prefetch_arrays();
    }
    
    if (program_blocks[last_program_block].worker_number == 0) {
        for (int i = last_program_block+1; i < program_blocks.size(); i ++) {
            if (program_blocks[i].worker_number != 0) {
                last_program_block = i;
                get_dead_arrays();
                block_map_.update_array_distance(program_blocks[i].array_distance);
                break;
            }
        }
    }
}

void ServerInterpreter::prefetch_arrays() {
    ProgramBlock &program_block = program_blocks[first_program_block];
    
    for (std::set<int>::iterator iter = program_block.get_arrays.begin(); 
              iter != program_block.get_arrays.end();
              iter ++) {
        if ( prefetched_arrays.count(*iter) == 0 ) {
            std::set<BlockId> blocks_set;
            block_map_.get_array_blocks(*iter, blocks_set);
            for (std::set<BlockId>::iterator itt = blocks_set.begin(); itt != blocks_set.end(); itt ++) {
                prefetching_blocks.insert(*itt);
            }
            prefetched_arrays.insert(*iter);
            //std::cout << "Prefetching array " << sip_tables_.array_name(*iter) << " ..." << std::endl;
        }
    }
}

void ServerInterpreter::remove_read_block(const BlockId &block_id) {
    prefetching_blocks.erase(block_id);
}

void ServerInterpreter::prefetch_block1() {
    std::set<BlockId>::iterator iter = prefetching_blocks.begin(); 
    
    if (iter != prefetching_blocks.end() && block_map_.is_block_prefetchable(*iter)) {
        block_map_.prefetch_block(*iter);
        prefetching_blocks.erase(iter);
    }
}

bool ServerInterpreter::more_blocks_to_prefetch() {
    return !prefetching_blocks.empty() 
            && block_map_.is_block_prefetchable(*prefetching_blocks.begin());
}

bool ServerInterpreter::more_blocks_to_free() {
    return !dead_blocks.empty();
}

bool ServerInterpreter::more_blocks_to_prefetch_or_free() {
    return more_blocks_to_prefetch() || more_blocks_to_free();
}

void ServerInterpreter::get_dead_arrays() {
    ProgramBlock &program_block = program_blocks[last_program_block];
    
    for (std::map<int, int>::iterator iter = program_block.array_distance.begin();
              iter != program_block.array_distance.end();
              iter ++) {
        if (iter->second == -1 && !dead_arrays.count(iter->first)) {
            std::set<BlockId> blocks_set;
            block_map_.get_dead_blocks(iter->first, blocks_set);
            for (std::set<BlockId>::iterator itt = blocks_set.begin(); itt != blocks_set.end(); itt ++) {
                dead_blocks.insert(*itt);
            }
            dead_arrays.insert(iter->first);
        }
    }
}

void ServerInterpreter::free_block1() {
    std::set<BlockId>::iterator iter = dead_blocks.begin(); 
    
    if (iter != dead_blocks.end()) {
        block_map_.free_block(*iter);
        prefetching_blocks.erase(iter);
    }
}

void ServerInterpreter::print_program_blocks() {

    std::cout << "Program Blocks: " << std::endl;
    for (int i = 0; i < program_blocks.size(); i ++) {
        std::cout << i << ": " << program_blocks[i].start_line_number << ", " << program_blocks[i].end_line_number << std::endl;
        std::cout << "Get arrays:" << std::endl;
        for (std::set<int>::iterator iter = program_blocks[i].get_arrays.begin();
                 iter != program_blocks[i].get_arrays.end();
                 iter ++) {
            std::cout << sip_tables_.array_name(*iter) << " ";
        }
        std::cout << std::endl;
        std::cout << "Put arrays:" << std::endl;
        for (std::set<int>::iterator iter = program_blocks[i].put_arrays.begin();
                 iter != program_blocks[i].put_arrays.end();
                 iter ++) {
            std::cout << sip_tables_.array_name(*iter) << " ";
        }
        std::cout << std::endl;
        std::cout << "Update arrays:" << std::endl;
        for (std::set<int>::iterator iter = program_blocks[i].update_arrays.begin();
                 iter != program_blocks[i].update_arrays.end();
                 iter ++) {
            std::cout << sip_tables_.array_name(*iter) << " ";
        }
        std::cout << std::endl;
        std::cout << "Array Distance:" << std::endl;
        for (std::map<int, int>::iterator iter = program_blocks[i].array_distance.begin();
                 iter != program_blocks[i].array_distance.end();
                 iter ++) {
            std::cout << sip_tables_.array_name(iter->first) << ":" << (iter->second) << " ";
        }
        std::cout << std::endl;
        std::cout << std::endl;
    }
    std::cout << std::endl;
}

void ServerInterpreter::generate_loop_blocks() {
    LoopBlock loop_block;
    bool do_sign = false;
    int nest_level = 0;
    int start_number = 0;
    
    std::set<int> arrays;
    
    loop_blocks.clear();
    
    for (int i = 0; i < op_table_.size(); i ++) {
        
        if (op_table_.opcode(i) == get_op) {
            loop_block.insert_get_array_reference(op_table_.arg0(i), op_table_.arg0(i-1), op_table_.index_selectors(i-1));
            //int array_id = op_table_.arg0(i);
            //std::cout << "Get operation: " << std::endl;
            //std::cout << "Array name: " << sip_tables_.array_name(array_id) << std::endl;
            //std::cout << "Index: ";
            //for (int j = 0; j < op_table_.arg0(i-1); j ++) {
            //    std::cout << sip_tables_.index_name(op_table_.index_selectors(i-1)[j]) << " ";
            //}
            //std::cout << std::endl;
            //std::cout << "Line number: " << op_table_.line_number(i) << std::endl;
        } else if (op_table_.opcode(i) == put_accumulate_op) {
            loop_block.insert_update_array_reference(op_table_.arg1(i-1), op_table_.arg0(i-1), op_table_.index_selectors(i-1));
            //int array_id = op_table_.arg1(i);
            //loop_block.update_arrays.insert(array_id);
            //arrays.insert(array_id);
        } /*else if (op_table_.opcode(i) == put_replace_op) {
            int array_id = op_table_.arg1(i);
            loop_block.put_arrays.insert(array_id);
            arrays.insert(array_id);
        }*/
        
        if (op_table_.opcode(i) == do_op) {
            nest_level ++;
            if (nest_level == 1 && do_sign == false) {
                loop_block = LoopBlock();
                loop_block.loop_type = DO;
                start_number = i;
                loop_block.indices.push_back(op_table_.index_selectors(i)[0]);
                loop_block.indices_index.insert(std::pair<int,int>(op_table_.index_selectors(i)[0],0));
                loop_block.worker_num[0] = SIPMPIAttr::get_instance().num_workers();
                do_sign = true;
            }
        }
        
        if (op_table_.opcode(i) == pardo_op) {
            nest_level ++;
            loop_block = LoopBlock();
            loop_block.loop_type = PARDO;
            start_number = i;
            for (int j = 0; j < op_table_.arg1(i); j ++) {
                loop_block.indices.push_back(op_table_.index_selectors(i)[j]);
                loop_block.indices_index.insert(std::pair<int,int>(op_table_.index_selectors(i)[j], j));
            }
            
            int *worker_num = loop_block.worker_num;
            worker_num[0] = SIPMPIAttr::get_instance().num_workers();
            
            int indice_start = 0;
            int seg_len = sip_tables_.num_segments(loop_block.indices[indice_start]);
            
            while (worker_num[indice_start] > seg_len) {
                worker_num[indice_start+1] = worker_num[indice_start] / seg_len;
                worker_num[indice_start] %= seg_len;
                indice_start++;
                if (indice_start >= op_table_.arg1(i)) {
                    break;
                }
                seg_len = sip_tables_.num_segments(loop_block.indices[indice_start]);
            }
    
            do_sign = true;
        }
        
        if (op_table_.opcode(i) == endpardo_op) {
            nest_level --;
            loop_block.start_line_number = op_table_.line_number(start_number);
            loop_block.end_line_number   = op_table_.line_number(i);
            loop_blocks.push_back(loop_block);
            do_sign = false;
        }
        
        if (op_table_.opcode(i) == enddo_op) {
            nest_level --;
            if (nest_level == 0 && do_sign == true) {
                loop_block.start_line_number = op_table_.line_number(start_number);
                loop_block.end_line_number   = op_table_.line_number(i);
                loop_blocks.push_back(loop_block);
                do_sign = false;
            }
        }
    }
}

int ServerInterpreter::get_block_index(int line_number) {
    int l = 0, h = loop_blocks.size() - 1;
    
    while (l < h) {
        int mid = (l + h) / 2;
    
        if (loop_blocks[mid].start_line_number <= line_number
            && loop_blocks[mid].end_line_number >= line_number) {
            return mid;
        } else if (loop_blocks[mid].start_line_number > line_number) {
            h = mid - 1;
        } else {
            l = mid + 1;
        }
    }
    
    return l;
}

void ServerInterpreter::handle_block_request(BlockId &block_id, int worker_rank, int line_number, 
                                             int *loop_indices, int indices_num) {

    int loop_index = get_block_index(line_number);
    LoopBlock &loop_block = loop_blocks[loop_index];
    int loop_rank = loop_block.indices.size();
    int indices_value[7] = {0};
    int *worker_num = loop_block.worker_num;
    
    int indice_start = 0;
    
    for (; indice_start < indices_num; indice_start ++) {
        if (loop_block.indices[0] == loop_indices[2*indice_start]) {
            break;
        }
    }
    
    for (int i = 0; i < loop_rank; i ++) {
        indices_value[i] = loop_indices[2*indice_start+2*i+1] 
                         - sip_tables_.lower_seg(loop_block.indices[i]);
    }
    
    std::list<WorkerIteration> &worker_iterations = workers_iteration[worker_rank];
    
    if (worker_iterations.size() != 0 
        && worker_iterations.begin()->loop_index != loop_index) {
        worker_iterations.clear();
    }
    
    for (std::list<WorkerIteration>::iterator iter = worker_iterations.begin(); 
              iter != worker_iterations.end(); 
              iter = worker_iterations.begin() ) {
        bool identical = true;
        for (int i = 0; i < 6; i ++) {
            if (iter->indices[i] != indices_value[i]) {
                identical = false;
            }
        }
        if (identical == false) {
            remove_worker_iteration(worker_iterations);
        } else {
            break;
        }
    }
    
    std::list<WorkerIteration>::iterator cur_iter = worker_iterations.begin();
    int current_iteration = 0, max_iteration = 4;
    bool more_iterations = worker_iterations.size() != max_iteration;
    
    while (more_iterations) {
        /*
        std::cout << "Loop iteration: ";
        for (int i = 0; i < loop_rank; i ++) {
            std::cout << sip_tables_.index_name(loop_block.indices[i]) << ": " 
            << (sip_tables_.lower_seg(loop_block.indices[i]) + indices_value[i]) << " ";
        }
        std::cout << std::endl;
        */
        
        if (cur_iter == worker_iterations.end()) {
            add_new_worker_iteration(loop_index, loop_block, indices_value, worker_iterations);
            cur_iter = worker_iterations.end();
        } else {
            cur_iter ++;
        }
        
        int carry = 0;
        
        for (int i = 0; i < loop_rank; i ++) {
            int seg_len = sip_tables_.num_segments(loop_block.indices[i]);
            if (indices_value[i] + worker_num[i] + carry >= seg_len) {
                indices_value[i] = (indices_value[i] + worker_num[i] + carry) % seg_len;
                carry = 1;
            } else {
                indices_value[i] = indices_value[i] + worker_num[i] + carry;
                carry = 0;
            }
        }
        
        indices_value[loop_rank] += worker_num[loop_rank] + carry;
        
        more_iterations = (++current_iteration < max_iteration) && (indices_value[loop_rank] == 0);
    }
    
    cur_iter = worker_iterations.begin();
    if (cur_iter != worker_iterations.end()) {
        if (cur_iter->blocks.count(block_id)) {
            cur_iter->blocks.erase(block_id);
            cur_iter->prefetch_blocks.erase(block_id);
            if (block_reference[block_id] != 0) {
                block_reference[block_id] --;
            }
        }
    }
    if (block_reference[block_id] == 0) {
        pending_dirty_blocks.erase(block_id);
        if (block_map_.is_block_dirty(block_id)) {
            dirty_blocks.insert(block_id);
        } else {
             unused_blocks.insert(block_id);
        }
    }
}

void ServerInterpreter::handle_put_block_request(BlockId &block_id, int worker_rank, int line_number) {
    if (block_reference[block_id] == 0) {
        pending_dirty_blocks.erase(block_id);
        if (block_map_.is_block_dirty(block_id)) {
            dirty_blocks.insert(block_id);
        } else {
            unused_blocks.insert(block_id);
        }
    }
}

void ServerInterpreter::add_new_worker_iteration(int loop_index, LoopBlock &loop_block, int indices_value[7],
                                                 std::list<WorkerIteration> &worker_iterations) {
    worker_iterations.push_back(WorkerIteration());
    WorkerIteration &worker_iteration = worker_iterations.back();
    
    worker_iteration.loop_index = loop_index;
    for (int i = 0; i < 6; i ++) {
        worker_iteration.indices[i] = indices_value[i];
    }
    
    for (std::vector<LoopBlock::ArrayReference>::iterator iter = loop_block.get_arrays.begin(); 
        iter != loop_block.get_arrays.end(); iter ++) {
        int index[6] = {-1,-1,-1,-1,-1,-1};
        
        for (int i = 0; i < iter->rank; i ++) {
            if (loop_block.indices_index.count(iter->index[i]) != 0) {
                index[i] = sip_tables_.lower_seg(iter->index[i]) + indices_value[loop_block.indices_index[iter->index[i]]];
            }
        }
        /*
        for (int i = 0; i < 6; i ++) {
            std::cout << index[i] << ", ";
        }
        std::cout << std::endl;
        */

        block_map_.select_blocks(iter->array_id, index, worker_iteration.blocks);
        /*
        for (auto iter = blocks.begin(); iter != blocks.end(); iter ++) {
            std::cout << *iter << std::endl;
        }
        */
    }
    /*
    for (auto iter = loop_block.update_arrays.begin(); iter != loop_block.get_arrays.end(); iter ++) {
        int index[6] = {-1,-1,-1,-1,-1,-1};
        
        for (int i = 0; i < iter->rank; i ++) {
            if (loop_block.indices_index.count(iter->index[i]) != 0) {
                index[i] = sip_tables_.lower_seg(iter->index[i]) + indices_value[loop_block.indices_index[iter->index[i]]];
            }
        }

        block_map_.select_blocks(iter->array_id, index, worker_iteration.blocks);
    }
    */
    std::set<BlockId> &blocks = worker_iteration.blocks;
    for (std::set<BlockId>::iterator iter = blocks.begin(); iter != blocks.end(); iter ++) {
        block_reference[*iter] ++;
        
        if (unused_blocks.count(*iter) != 0) {
            unused_blocks.erase(*iter);
        }
        
        if (dirty_blocks.count(*iter) != 0) {
            dirty_blocks.erase(*iter);
        }
        
        if (!block_map_.is_block_in_memory(*iter)) {
            worker_iteration.prefetch_blocks.insert(*iter);
        } else if (!block_map_.is_block_dirty(*iter)) {
            pending_dirty_blocks.insert(*iter);
        }
    }
}

void ServerInterpreter::remove_worker_iteration(std::list<WorkerIteration> &worker_iterations) {
    WorkerIteration &worker_iteration = worker_iterations.front();
    
    std::set<BlockId> &blocks = worker_iteration.blocks;
    for (std::set<BlockId>::iterator iter = blocks.begin(); iter != blocks.end(); iter ++) {
        block_reference[*iter] --;
        if (block_reference[*iter] == 0) {
            if (block_map_.is_block_dirty(*iter)) {
                dirty_blocks.insert(*iter);
            } else {
                 unused_blocks.insert(*iter);
            }
            pending_dirty_blocks.erase(*iter);
        }
    }
    
    worker_iterations.pop_front();
}

bool ServerInterpreter::prefetch_block() {
    for (std::map<int, std::list<WorkerIteration> >::iterator iter = workers_iteration.begin(); 
        iter != workers_iteration.end(); iter ++) {
        if (iter->second.size() == 0) {
            continue;
        }
        
        std::list<WorkerIteration>::iterator it = iter->second.begin();
        {
            std::set<BlockId> &prefetch_blocks = it->prefetch_blocks;
            
            if (!prefetch_blocks.empty()) {
                while (!prefetch_blocks.empty()) {
                    if (block_map_.prefetch_block(*(prefetch_blocks.begin()))) {
                        prefetch_blocks.erase(prefetch_blocks.begin());
                        return true;
                    }
                    prefetch_blocks.erase(prefetch_blocks.begin());
                }
            }
        }
        
        it ++;
        
        if (it != iter->second.end())
        {
            std::set<BlockId> &prefetch_blocks = it->prefetch_blocks;
            
            if (!prefetch_blocks.empty()) {
                while (!prefetch_blocks.empty()) {
                    if (block_map_.prefetch_block(*(prefetch_blocks.begin()))) {
                        prefetch_blocks.erase(prefetch_blocks.begin());
                        return true;
                    }
                    prefetch_blocks.erase(prefetch_blocks.begin());
                }
            }
        }
    }
    
    return false;
}

bool ServerInterpreter::write_block() {
    while (dirty_blocks.begin() != dirty_blocks.end()) {
        std::set<BlockId>::iterator iter = dirty_blocks.begin();
        bool sign = block_map_.write_block(*iter);
        
        dirty_blocks.erase(*iter);
        unused_blocks.insert(*iter);
        
        if (sign) {
            return true;
        }
    }
    
    while (pending_dirty_blocks.begin() != pending_dirty_blocks.end()) {
        std::set<BlockId>::iterator iter = pending_dirty_blocks.begin();
        bool sign = block_map_.write_block(*iter);
        
        pending_dirty_blocks.erase(*iter);
        
        if (sign) {
            return true;
        }
    }
    
    return false;
}

void ServerInterpreter::print_workers_iterations() {
    std::cout << "Workers Iterations:" << std::endl;
    
    for (std::map<int, std::list<WorkerIteration> >::iterator iter = workers_iteration.begin(); 
              iter != workers_iteration.end();
              iter ++) {
        std::cout << "Worker: " << iter->first << std::endl;
        for (std::list<WorkerIteration>::iterator it = iter->second.begin();
                  it != iter->second.end(); it ++) {
            LoopBlock &loop_block = loop_blocks[it->loop_index];
            std::cout << "Iteration: ";
            for (int i = 0; i < loop_block.indices.size(); i ++) {
                std::cout << sip_tables_.index_name(loop_block.indices[i]) << ": " 
                << (sip_tables_.lower_seg(loop_block.indices[i]) + it->indices[i]) << " ";
            }
            std::cout << std::endl;
            
            std::cout << "Blocks:" << std::endl;
            for (std::set<BlockId>::iterator itt = it->blocks.begin(); itt != it->blocks.end(); itt ++) {
                std::cout << *itt << std::endl;
            }
            
            std::cout << "Prefetch Blocks:" << std::endl;
            for (std::set<BlockId>::iterator itt = it->prefetch_blocks.begin(); itt != it->prefetch_blocks.end(); itt ++) {
                std::cout << *itt << std::endl;
            }
            
        }
    }

}

void ServerInterpreter::print_loop_blocks() {

    std::cout << "Loop Blocks: " << std::endl;
    for (int i = 0; i < loop_blocks.size(); i ++) {
        std::cout << (loop_blocks[i].loop_type == DO?"Do":"Pardo") << "Loop: " << i << ": " << loop_blocks[i].start_line_number << ", " << loop_blocks[i].end_line_number << std::endl;
        std::cout << "Get arrays:" << std::endl;
        for (std::vector<LoopBlock::ArrayReference>::iterator iter = loop_blocks[i].get_arrays.begin();
                 iter != loop_blocks[i].get_arrays.end();
                 iter ++) {
            std::cout << "Array name: " << sip_tables_.array_name(iter->array_id) << " , Rank: " << iter->rank << std::endl;
            std::cout << "Index: ";
            for (int j = 0; j < iter->rank; j ++) {
                std::cout << sip_tables_.index_name(iter->index[j]) << " ";
            }
            std::cout << std::endl;
        }
        std::cout << std::endl;
    }
    std::cout << std::endl;
}

void ServerInterpreter::print_op_table() {
	std::cout <<  "Optable: " << op_table_.size() << " opcodes." << std::endl;
    
    for (int i = 0; i < op_table_.size(); i ++) {
        std::cout << "Opcode: " << opcodeToName(op_table_.opcode(i)) << std::endl;
        std::cout << "arg: " << op_table_.arg0(i) << " " << op_table_.arg1(i) << " " << op_table_.arg2(i) << std::endl;
        for (int j = 0; j < MAX_RANK; j ++) {
            std::cout << op_table_.index_selectors(i)[j] << " ";
        }
        std::cout << std::endl;
        std::cout << op_table_.line_number(i);
        std::cout << std::endl << std::endl;
    }
}

}/* namespace sip */

