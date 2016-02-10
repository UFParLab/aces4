/*
 * loop_manager.cpp
 *
 *  Created on: Aug 23, 2013
 *      Author: basbas
 */

#include "fragment_loop_manager.h"
#include <sstream>
#include "interpreter.h"

#ifdef HAVE_MPI

namespace sip {

FragmentPardoLoopManager::FragmentPardoLoopManager(int num_indices,
		const int (&index_id)[MAX_RANK], DataManager & data_manager,
		const SipTables & sip_tables) :
		data_manager_(data_manager), sip_tables_(sip_tables), num_indices_(num_indices)  {
	std::copy(index_id + 0, index_id + MAX_RANK, index_id_ + 0);
	for (int i = 0; i < num_indices; ++i) {
		lower_seg_[i] = sip_tables_.lower_seg(index_id_[i]);
		upper_bound_[i] = lower_seg_[i]
				+ sip_tables_.num_segments(index_id_[i]);
		CHECK_WITH_LINE(lower_seg_[i] < upper_bound_[i],
				"Pardo loop index " + sip_tables_.index_name(index_id_[i])
						+ " has empty range",
				Interpreter::global_interpreter->line_number());
	}
}

void FragmentPardoLoopManager::do_finalize() {
	for (int i = 0; i < num_indices_; ++i) {
		data_manager_.set_index_undefined(index_id_[i]);
	}
}

bool FragmentPardoLoopManager::initialize_indices() {
	//initialize values of all indices
	bool more_iterations = true;
	for (int i = 0; i < num_indices_; ++i) {
		if (lower_seg_[i] >= upper_bound_[i]) {
			more_iterations = false; //this loop has an empty range in at least one dimension.
			return more_iterations;
		}
		CHECK_WITH_LINE(
				data_manager_.index_value(index_id_[i])
						== DataManager::undefined_index_value,
				"SIAL or SIP error, index "
						+ sip_tables_.index_name(index_id_[i])
						+ " already has value before loop",
				Interpreter::global_interpreter->line_number());
		index_values_[i] = lower_seg_[i];
		data_manager_.set_index_value(index_id_[i], lower_seg_[i]);
	}
	more_iterations = true;
	return more_iterations;
}

bool FragmentPardoLoopManager::increment_simple_pair() {
	bool more = false;      // More iterations?
	int current_value;
	for (int i = 0; i < 2; ++i) {
		more = increment_single_index(i);
		if (more) {
			break;
		}
	} //if here, then all indices are at their max value
	return more;
}

bool FragmentPardoLoopManager::increment_single_index(int index) {
	bool more = false; 	// More iterations?
	int current_value;
	current_value = data_manager_.index_value(index_id_[index]);
	++current_value;
	if (current_value < upper_bound_[index]) {
		//increment current index and return
		index_values_[index] = current_value;
		data_manager_.set_index_value(index_id_[index], current_value);
		more = true;
	} else {
		//wrap around and handle next index
		index_values_[index] = lower_seg_[index];
		data_manager_.set_index_value(index_id_[index], lower_seg_[index]);
	}
	return more;
}

bool FragmentPardoLoopManager::increment_all() {
	bool more = false;      // More iterations?
	int current_value;
	for (int i = 0; i < num_indices_; ++i) {
		more = increment_single_index(i);
		if (more) {
			break;
		}
	} //if here, then all indices are at their max value
	return more;
}

void FragmentPardoLoopManager::form_rcut_dist() {
	int rcut_dist_array_slot = sip_tables_.array_slot(std::string("rcut_dist"));
	Block::BlockPtr bptr_rcut_dist =
			data_manager_.contiguous_array_manager().get_array(
					rcut_dist_array_slot);
	double *val_rcut_dist = bptr_rcut_dist->get_data();
	int rcut_dist_size = bptr_rcut_dist->size();
	int num_frag = upper_bound_[0] - lower_seg_[0];
	rcut_dist.resize(num_frag);
	for (int i = 0; i < num_frag; ++i) {
		//std::cout << i << std::endl;
		rcut_dist[i].resize(num_frag);
	}
	int ij = 0;
	for (int i = 0; i < num_frag; ++i) {
		for (int j = 0; j < num_frag; ++j) {
			rcut_dist[j][i] = (int) val_rcut_dist[ij];
			//std::cout << i << " " << j << " " << ij << " " << rcut_dist[j][i] << " " << val_rcut_dist[ij]<< std::endl;
			++ij;
		}
	}
	return;
}

void FragmentPardoLoopManager::form_elst_dist() {
	int elst_dist_array_slot = sip_tables_.array_slot(std::string("elst_dist"));
	Block::BlockPtr bptr_elst_dist =
			data_manager_.contiguous_array_manager().get_array(
					elst_dist_array_slot);
	double *val_elst_dist = bptr_elst_dist->get_data();
	int elst_dist_size = bptr_elst_dist->size();
	int num_frag = upper_bound_[0] - lower_seg_[0];
	elst_dist.resize(num_frag);
	for (int i = 0; i < num_frag; ++i) {
		//std::cout << i << std::endl;
		elst_dist[i].resize(num_frag);
	}
	int ij = 0;
	for (int i = 0; i < num_frag; ++i) {
		for (int j = 0; j < num_frag; ++j) {
			elst_dist[j][i] = (int) val_elst_dist[ij];
			//std::cout << i << " " << j << " " << ij << " " << elst_dist[j][i] << " " << val_elst_dist[ij]<< std::endl;
			++ij;
		}
	}
	return;
}

void FragmentPardoLoopManager::form_swao_frag() {
	int swao_frag_array_slot = sip_tables_.array_slot(std::string("swao_frag"));
	Block::BlockPtr bptr_swao_frag =
			data_manager_.contiguous_array_manager().get_array(
					swao_frag_array_slot);
	double *val_swao_frag = bptr_swao_frag->get_data();
	int swao_frag_size = bptr_swao_frag->size();
	swao_frag.resize(swao_frag_size);
	for (int i = 0; i < swao_frag_size; ++i) {
		swao_frag[i] = (int) val_swao_frag[i];
		//std::cout << "vec " << swao_frag[i] << " data " << val_swao_frag[i] << std::endl;
	}
	return;
}
    
    
    void FragmentPardoLoopManager::form_swmoa_frag() {
        int swmoa_frag_array_slot = sip_tables_.array_slot(std::string("swmoa_frag"));
        Block::BlockPtr bptr_swmoa_frag =
        data_manager_.contiguous_array_manager().get_array(
                                                           swmoa_frag_array_slot);
        double *val_swmoa_frag = bptr_swmoa_frag->get_data();
        int swmoa_frag_size = bptr_swmoa_frag->size();
        swmoa_frag.resize(swmoa_frag_size);
        for (int i = 0; i < swmoa_frag_size; ++i) {
            swmoa_frag[i] = (int) val_swmoa_frag[i];
            //std::cout << "vec " << swmoa_frag[i] << " data " << val_swmoa_frag[i] << std::endl;
        }
        return;
    }

void FragmentPardoLoopManager::form_swocca_frag() {
	int swocca_frag_array_slot = sip_tables_.array_slot(
			std::string("swocca_frag"));
	Block::BlockPtr bptr_swocca_frag =
			data_manager_.contiguous_array_manager().get_array(
					swocca_frag_array_slot);
	double *val_swocca_frag = bptr_swocca_frag->get_data();
	int swocca_frag_size = bptr_swocca_frag->size();
	swocca_frag.resize(swocca_frag_size);
	for (int i = 0; i < swocca_frag_size; ++i) {
		swocca_frag[i] = (int) val_swocca_frag[i];
		//std::cout << "vec " << swocca_frag[i] << " data " << val_swocca_frag[i] << std::endl;
	}
	return;
}

void FragmentPardoLoopManager::form_swvirta_frag() {
	int swvirta_frag_array_slot = sip_tables_.array_slot(
			std::string("swvirta_frag"));
	Block::BlockPtr bptr_swvirta_frag =
			data_manager_.contiguous_array_manager().get_array(
					swvirta_frag_array_slot);
	double *val_swvirta_frag = bptr_swvirta_frag->get_data();
	int swvirta_frag_size = bptr_swvirta_frag->size();
	swvirta_frag.resize(swvirta_frag_size);
	for (int i = 0; i < swvirta_frag_size; ++i) {
		swvirta_frag[i] = (int) val_swvirta_frag[i];
		//std::cout << "vec " << swvirta_frag[i] << " data " << val_swvirta_frag[i] << std::endl;
	}
	return;
}

bool FragmentPardoLoopManager::fragment_special_where_clause(int typ, int index,
		int frag) {
	bool where_clause;
	int ij = 0;
	switch (typ) {

	case 1: // check against ao index
		//where_clause = return_val_swao_frag(index_values_[index])     == index_values_[frag];
		where_clause = swao_frag[index_values_[index] - lower_seg_[index]]
				== index_values_[frag];
		break;

	case 2: // check against occ index
		//where_clause = return_val_swocca_frag(index_values_[index])   == index_values_[frag];
		where_clause = swocca_frag[index_values_[index] - lower_seg_[index]]
				== index_values_[frag];
		break;

	case 3: // check against virt index
		//where_clause = return_val_swvirta_frag(index_values_[index])  == index_values_[frag];
		where_clause = swvirta_frag[index_values_[index] - lower_seg_[index]]
				== index_values_[frag];
		break;

	case 4: // check elst_dist[ifrag,jfrag] == ifrag
		//where_clause = return_val_elst_dist(index_values_[frag], index_values_[index]) == index_values_[frag];
		//ij = index_values_[frag] - 1 + (upper_bound_[0])*(index_values_[index] - 1);
		//std::cout << index_values_[index] << " " << index_values_[frag] << " " << ij << " " << elst_dist[ij] << std::endl;
		//where_clause = elst_dist[ij] == index_values_[frag];
		where_clause =
				elst_dist[index_values_[frag] - lower_seg_[frag]][index_values_[index]
						- lower_seg_[index]] == index_values_[frag];
		break;

	case 5: // check rcut_dist[ifrag,jfrag] == ifrag
		//where_clause = return_val_rcut_dist(index_values_[frag], index_values_[index]) == index_values_[frag];
		//ij = index_values_[index] - 1 + upper_bound_[0]*(index_values_[frag] - 1);
		//where_clause = rcut_dist[ij] == index_values_[frag];
		//break;
		where_clause =
				rcut_dist[index_values_[frag] - lower_seg_[frag]][index_values_[index]
						- lower_seg_[index]] == index_values_[frag];
		break;

	case 6: // check SwMOA_frag[ifrag] == ifrag
		where_clause = swmoa_frag[index_values_[index] - lower_seg_[index]]
				== index_values_[frag];
		break;

	case 0:
		where_clause = index_values_[frag] != index_values_[index];
		break;
	default:
		where_clause = false;
	}
	//std::cout << "where_clause " << typ << " " << index_values_[index] << " " << index_values_[frag] << " " << where_clause << std::endl;
	return where_clause;
}

std::string FragmentPardoLoopManager::to_string() const {
	std::stringstream ss;
	ss << "Balanced Task Allocation Parallel Pardo Loop:  num_indices="
			<< num_indices_ << std::endl;
	ss << "index_ids_=[";
	for (int i = 0; i < num_indices_; ++i) {
		ss << (i == 0 ? "" : ",") << sip_tables_.index_name(index_id_[i]);
	}
	ss << "] lower_seg_=[";
	for (int i = 0; i < num_indices_; ++i) {
		ss << (i == 0 ? "" : ",") << lower_seg_[i];
	}
	ss << "] upper_bound_=[";
	for (int i = 0; i < num_indices_; ++i) {
		ss << (i == 0 ? "" : ",") << upper_bound_[i];
	}
	ss << "] current= [";
	for (int i = 0; i < num_indices_; ++i) {
		ss << (i == 0 ? "" : ",")
				<< data_manager_.index_value_to_string(index_id_[i]);
	}
	ss << "]";
	return ss.str();
}

// ----------------------------------------------------------------------------------------
/*!
 Fragment_ special naming scheme: _<simple index fragment indices>_<occ (o), virt (v), ao (a) in ifrag>_<o,v,a in jfrag>
 */

/*!
 -------------------------------------------
 _Nij_aa__
 Frag{Nij}{aa}{}
 -------------------------------------------
 */

/*
 for each new special fragment where clause pattern, this should be the only thing realy changed.
 see comment above for fragment_special_where_clause syntax
 */
bool Fragment_Nij_aa__PardoLoopManager::where_clause(int index) {
	bool where_;
	int ifrag = 0;
	int jfrag = 1;
	int ao = 1;
	int occ = 2;
	int virt = 3;
	int elst = 4;
	int rcut = 5;
	int NE = 0;

	switch (index) {
	case 0:
	case 1:
		where_ = fragment_special_where_clause(elst, jfrag, ifrag)
				&& fragment_special_where_clause(NE, jfrag, ifrag);
		break;
	case 2:
		where_ = fragment_special_where_clause(ao, index, ifrag);
		break;
	case 3:
		where_ = fragment_special_where_clause(ao, index, ifrag);
		break;
	default:
		where_ = false;
	}
	return where_;
}

bool Fragment_Nij_aa__PardoLoopManager::do_update() {
	if (to_exit_)
		return false;
	bool more_iters;
	bool where_clauses_value;

	interpreter_->skip_where_clauses(num_where_clauses_);

	int loop_count = iteration_;
	int index_restart[MAX_RANK];

	if (first_time_) {
		first_time_ = false;
		more_iters = initialize_indices();

		for (int i = 0; i < num_indices_; ++i) {
			index_restart[i] = index_values_[i];
		}

		for (int index_i = index_restart[0]; index_i < upper_bound_[0];
				++index_i) {
			index_values_[0] = index_i;
			data_manager_.set_index_value(index_id_[0], index_i);

			for (int index_j = index_restart[1]; index_j < upper_bound_[1];
					++index_j) {
				index_values_[1] = index_j;
				data_manager_.set_index_value(index_id_[1], index_j);

				if (where_clause(1)) {
					for (int index_2 = index_restart[2];
							index_2 < upper_bound_[2]; ++index_2) {
						index_values_[2] = index_2;
						data_manager_.set_index_value(index_id_[2], index_2);

						if (where_clause(2)) {
							for (int index_3 = index_restart[3];
									index_3 < upper_bound_[3]; ++index_3) {
								index_values_[3] = index_3;
								data_manager_.set_index_value(index_id_[3],
										index_3);

								if (where_clause(3)) {
									iteration_++;
									if ((iteration_ - 1) % num_workers_
											== company_rank_) {
										return true;
									}
								} // where 3
							} // index_3
							for (int i = 3; i < num_indices_; ++i) {
								index_restart[i] = lower_seg_[i];
							}
						} // where 2
					} // index_2
					for (int i = 2; i < num_indices_; ++i) {
						index_restart[i] = lower_seg_[i];
					}
				} // where 1
			} // index_j
			for (int i = 1; i < num_indices_; ++i) {
				index_restart[i] = lower_seg_[i];
			}
		} // index_i
	}

	for (int i = 0; i < num_indices_; ++i) {
		index_restart[i] = index_values_[i];
	}

	for (int index_i = index_restart[0]; index_i < upper_bound_[0]; ++index_i) {
		index_values_[0] = index_i;
		data_manager_.set_index_value(index_id_[0], index_i);

		for (int index_j = index_restart[1]; index_j < upper_bound_[1];
				++index_j) {
			index_values_[1] = index_j;
			data_manager_.set_index_value(index_id_[1], index_j);

			if (where_clause(1)) {
				for (int index_2 = index_restart[2]; index_2 < upper_bound_[2];
						++index_2) {
					index_values_[2] = index_2;
					data_manager_.set_index_value(index_id_[2], index_2);

					if (where_clause(2)) {
						for (int index_3 = index_restart[3];
								index_3 < upper_bound_[3]; ++index_3) {
							index_values_[3] = index_3;
							data_manager_.set_index_value(index_id_[3],
									index_3);

							if (where_clause(3)) {
								if (loop_count > iteration_) {
									iteration_++;
									if ((iteration_ - 1) % num_workers_
											== company_rank_) {
										return true;
									}
								}
								++loop_count;
							} // where 3
						} // index_3
						for (int i = 3; i < num_indices_; ++i) {
							index_restart[i] = lower_seg_[i];
						}
					} // where 2
				} // index_2
				for (int i = 2; i < num_indices_; ++i) {
					index_restart[i] = lower_seg_[i];
				}
			} // where 1
		} // index_j
		for (int i = 1; i < num_indices_; ++i) {
			index_restart[i] = lower_seg_[i];
		}
	} // index_i

	return false; //this should be false here
}

Fragment_Nij_aa__PardoLoopManager::Fragment_Nij_aa__PardoLoopManager(
		int num_indices, const int (&index_id)[MAX_RANK],
		DataManager & data_manager, const SipTables & sip_tables,
		SIPMPIAttr & sip_mpi_attr, int num_where_clauses,
		Interpreter* interpreter, long& iteration) :
		FragmentPardoLoopManager(num_indices, index_id, data_manager,
				sip_tables), first_time_(true), iteration_(iteration), sip_mpi_attr_(
				sip_mpi_attr), num_where_clauses_(num_where_clauses), company_rank_(
				sip_mpi_attr.company_rank()), num_workers_(
				sip_mpi_attr_.num_workers()), interpreter_(interpreter) {

	form_elst_dist();
	//form_rcut_dist();
	form_swao_frag();
	//form_swocca_frag();
	//form_swvirta_frag();
}

Fragment_Nij_aa__PardoLoopManager::~Fragment_Nij_aa__PardoLoopManager() {
}

/*!
 -------------------------------------------
 _Nij_a_a_
 -------------------------------------------
 */

/*
 for each new special fragment where clause pattern, this should be the only thing realy changed.
 see comment above for fragment_special_where_clause syntax
 */
bool Fragment_Nij_a_a_PardoLoopManager::where_clause(int index) {
	bool where_;
	int ifrag = 0;
	int jfrag = 1;
	int ao = 1;
	int occ = 2;
	int virt = 3;
	int elst = 4;
	int rcut = 5;
	int NE = 0;

	switch (index) {
	case 0:
	case 1:
		where_ = fragment_special_where_clause(elst, jfrag, ifrag)
				&& fragment_special_where_clause(NE, jfrag, ifrag);
		break;
	case 2:
		where_ = fragment_special_where_clause(ao, index, ifrag);
		break;
	case 3:
		where_ = fragment_special_where_clause(ao, index, jfrag);
		break;
	default:
		where_ = false;
	}
	return where_;
}

bool Fragment_Nij_a_a_PardoLoopManager::do_update() {
	if (to_exit_)
		return false;
	bool more_iters;
	bool where_clauses_value;

	interpreter_->skip_where_clauses(num_where_clauses_);

	int loop_count = iteration_;
	int index_restart[MAX_RANK];

	if (first_time_) {
		first_time_ = false;
		more_iters = initialize_indices();

		for (int i = 0; i < num_indices_; ++i) {
			index_restart[i] = index_values_[i];
		}

		for (int index_i = index_restart[0]; index_i < upper_bound_[0];
				++index_i) {
			index_values_[0] = index_i;
			data_manager_.set_index_value(index_id_[0], index_i);

			for (int index_j = index_restart[1]; index_j < upper_bound_[1];
					++index_j) {
				index_values_[1] = index_j;
				data_manager_.set_index_value(index_id_[1], index_j);

				if (where_clause(1)) {
					for (int index_2 = index_restart[2];
							index_2 < upper_bound_[2]; ++index_2) {
						index_values_[2] = index_2;
						data_manager_.set_index_value(index_id_[2], index_2);

						if (where_clause(2)) {
							for (int index_3 = index_restart[3];
									index_3 < upper_bound_[3]; ++index_3) {
								index_values_[3] = index_3;
								data_manager_.set_index_value(index_id_[3],
										index_3);

								if (where_clause(3)) {
									iteration_++;
									if ((iteration_ - 1) % num_workers_
											== company_rank_) {
										return true;
									}
								} // where 3
							} // index_3
							for (int i = 3; i < num_indices_; ++i) {
								index_restart[i] = lower_seg_[i];
							}
						} // where 2
					} // index_2
					for (int i = 2; i < num_indices_; ++i) {
						index_restart[i] = lower_seg_[i];
					}
				} // where 1
			} // index_j
			for (int i = 1; i < num_indices_; ++i) {
				index_restart[i] = lower_seg_[i];
			}
		} // index_i
	}

	for (int i = 0; i < num_indices_; ++i) {
		index_restart[i] = index_values_[i];
	}

	for (int index_i = index_restart[0]; index_i < upper_bound_[0]; ++index_i) {
		index_values_[0] = index_i;
		data_manager_.set_index_value(index_id_[0], index_i);

		for (int index_j = index_restart[1]; index_j < upper_bound_[1];
				++index_j) {
			index_values_[1] = index_j;
			data_manager_.set_index_value(index_id_[1], index_j);

			if (where_clause(1)) {
				for (int index_2 = index_restart[2]; index_2 < upper_bound_[2];
						++index_2) {
					index_values_[2] = index_2;
					data_manager_.set_index_value(index_id_[2], index_2);

					if (where_clause(2)) {
						for (int index_3 = index_restart[3];
								index_3 < upper_bound_[3]; ++index_3) {
							index_values_[3] = index_3;
							data_manager_.set_index_value(index_id_[3],
									index_3);

							if (where_clause(3)) {
								if (loop_count > iteration_) {
									iteration_++;
									if ((iteration_ - 1) % num_workers_
											== company_rank_) {
										return true;
									}
								}
								++loop_count;
							} // where 3
						} // index_3
						for (int i = 3; i < num_indices_; ++i) {
							index_restart[i] = lower_seg_[i];
						}
					} // where 2
				} // index_2
				for (int i = 2; i < num_indices_; ++i) {
					index_restart[i] = lower_seg_[i];
				}
			} // where 1
		} // index_j
		for (int i = 1; i < num_indices_; ++i) {
			index_restart[i] = lower_seg_[i];
		}
	} // index_i

	return false; //this should be false here
}

Fragment_Nij_a_a_PardoLoopManager::Fragment_Nij_a_a_PardoLoopManager(
		int num_indices, const int (&index_id)[MAX_RANK],
		DataManager & data_manager, const SipTables & sip_tables,
		SIPMPIAttr & sip_mpi_attr, int num_where_clauses,
		Interpreter* interpreter, long& iteration) :
		FragmentPardoLoopManager(num_indices, index_id, data_manager,
				sip_tables), first_time_(true), iteration_(iteration), sip_mpi_attr_(
				sip_mpi_attr), num_where_clauses_(num_where_clauses), company_rank_(
				sip_mpi_attr.company_rank()), num_workers_(
				sip_mpi_attr_.num_workers()), interpreter_(interpreter) {

	form_elst_dist();
	//form_rcut_dist();
	form_swao_frag();
	//form_swocca_frag();
	//form_swvirta_frag();
}

Fragment_Nij_a_a_PardoLoopManager::~Fragment_Nij_a_a_PardoLoopManager() {
}

/*!
 -------------------------------------------
 _Nij_oo__
 Frag{Nij}{oo}{}
 -------------------------------------------
 */

/*
 for each new special fragment where clause pattern, this should be the only thing realy changed.
 see comment above for fragment_special_where_clause syntax
 */
bool Fragment_Nij_oo__PardoLoopManager::where_clause(int index) {
	bool where_;
	int ifrag = 0;
	int jfrag = 1;
	int ao = 1;
	int occ = 2;
	int virt = 3;
	int elst = 4;
	int rcut = 5;
	int NE = 0;

	switch (index) {
	case 0:
	case 1:
		where_ = fragment_special_where_clause(elst, jfrag, ifrag)
				&& fragment_special_where_clause(NE, jfrag, ifrag);
		break;
	case 2:
		where_ = fragment_special_where_clause(occ, index, ifrag);
		break;
	case 3:
		where_ = fragment_special_where_clause(occ, index, ifrag);
		break;
	default:
		where_ = false;
	}
	return where_;
}

bool Fragment_Nij_oo__PardoLoopManager::do_update() {
	if (to_exit_)
		return false;
	bool more_iters;
	bool where_clauses_value;

	interpreter_->skip_where_clauses(num_where_clauses_);

	int loop_count = iteration_;
	int index_restart[MAX_RANK];

	if (first_time_) {
		first_time_ = false;
		more_iters = initialize_indices();

		for (int i = 0; i < num_indices_; ++i) {
			index_restart[i] = index_values_[i];
		}

		for (int index_i = index_restart[0]; index_i < upper_bound_[0];
				++index_i) {
			index_values_[0] = index_i;
			data_manager_.set_index_value(index_id_[0], index_i);

			for (int index_j = index_restart[1]; index_j < upper_bound_[1];
					++index_j) {
				index_values_[1] = index_j;
				data_manager_.set_index_value(index_id_[1], index_j);

				if (where_clause(1)) {
					for (int index_2 = index_restart[2];
							index_2 < upper_bound_[2]; ++index_2) {
						index_values_[2] = index_2;
						data_manager_.set_index_value(index_id_[2], index_2);

						if (where_clause(2)) {
							for (int index_3 = index_restart[3];
									index_3 < upper_bound_[3]; ++index_3) {
								index_values_[3] = index_3;
								data_manager_.set_index_value(index_id_[3],
										index_3);

								if (where_clause(3)) {
									iteration_++;
									if ((iteration_ - 1) % num_workers_
											== company_rank_) {
										return true;
									}
								} // where 3
							} // index_3
							for (int i = 3; i < num_indices_; ++i) {
								index_restart[i] = lower_seg_[i];
							}
						} // where 2
					} // index_2
					for (int i = 2; i < num_indices_; ++i) {
						index_restart[i] = lower_seg_[i];
					}
				} // where 1
			} // index_j
			for (int i = 1; i < num_indices_; ++i) {
				index_restart[i] = lower_seg_[i];
			}
		} // index_i
	}

	for (int i = 0; i < num_indices_; ++i) {
		index_restart[i] = index_values_[i];
	}

	for (int index_i = index_restart[0]; index_i < upper_bound_[0]; ++index_i) {
		index_values_[0] = index_i;
		data_manager_.set_index_value(index_id_[0], index_i);

		for (int index_j = index_restart[1]; index_j < upper_bound_[1];
				++index_j) {
			index_values_[1] = index_j;
			data_manager_.set_index_value(index_id_[1], index_j);

			if (where_clause(1)) {
				for (int index_2 = index_restart[2]; index_2 < upper_bound_[2];
						++index_2) {
					index_values_[2] = index_2;
					data_manager_.set_index_value(index_id_[2], index_2);

					if (where_clause(2)) {
						for (int index_3 = index_restart[3];
								index_3 < upper_bound_[3]; ++index_3) {
							index_values_[3] = index_3;
							data_manager_.set_index_value(index_id_[3],
									index_3);

							if (where_clause(3)) {
								if (loop_count > iteration_) {
									iteration_++;
									if ((iteration_ - 1) % num_workers_
											== company_rank_) {
										return true;
									}
								}
								++loop_count;
							} // where 3
						} // index_3
						for (int i = 3; i < num_indices_; ++i) {
							index_restart[i] = lower_seg_[i];
						}
					} // where 2
				} // index_2
				for (int i = 2; i < num_indices_; ++i) {
					index_restart[i] = lower_seg_[i];
				}
			} // where 1
		} // index_j
		for (int i = 1; i < num_indices_; ++i) {
			index_restart[i] = lower_seg_[i];
		}
	} // index_i

	return false; //this should be false here
}

Fragment_Nij_oo__PardoLoopManager::Fragment_Nij_oo__PardoLoopManager(
		int num_indices, const int (&index_id)[MAX_RANK],
		DataManager & data_manager, const SipTables & sip_tables,
		SIPMPIAttr & sip_mpi_attr, int num_where_clauses,
		Interpreter* interpreter, long& iteration) :
		FragmentPardoLoopManager(num_indices, index_id, data_manager,
				sip_tables), first_time_(true), iteration_(iteration), sip_mpi_attr_(
				sip_mpi_attr), num_where_clauses_(num_where_clauses), company_rank_(
				sip_mpi_attr.company_rank()), num_workers_(
				sip_mpi_attr_.num_workers()), interpreter_(interpreter) {

	form_elst_dist();
	//form_rcut_dist();
	//form_swao_frag();
	form_swocca_frag();
	//form_swvirta_frag();
}

Fragment_Nij_oo__PardoLoopManager::~Fragment_Nij_oo__PardoLoopManager() {
}

/*!
 -------------------------------------------
 _Nij_o_o_
 Frag{Nij}{o}{o}
 -------------------------------------------
 */

/*
 for each new special fragment where clause pattern, this should be the only thing realy changed.
 see comment above for fragment_special_where_clause syntax
 */
bool Fragment_Nij_o_o_PardoLoopManager::where_clause(int index) {
	bool where_;
	int ifrag = 0;
	int jfrag = 1;
	int ao = 1;
	int occ = 2;
	int virt = 3;
	int elst = 4;
	int rcut = 5;
	int NE = 0;

	switch (index) {
	case 0:
	case 1:
		where_ = fragment_special_where_clause(elst, jfrag, ifrag)
				&& fragment_special_where_clause(NE, jfrag, ifrag);
		break;
	case 2:
		where_ = fragment_special_where_clause(occ, index, ifrag);
		break;
	case 3:
		where_ = fragment_special_where_clause(occ, index, jfrag);
		break;
	default:
		where_ = false;
	}
	return where_;
}

bool Fragment_Nij_o_o_PardoLoopManager::do_update() {
	if (to_exit_)
		return false;
	bool more_iters;
	bool where_clauses_value;

	interpreter_->skip_where_clauses(num_where_clauses_);

	int loop_count = iteration_;
	int index_restart[MAX_RANK];

	if (first_time_) {
		first_time_ = false;
		more_iters = initialize_indices();

		for (int i = 0; i < num_indices_; ++i) {
			index_restart[i] = index_values_[i];
		}

		for (int index_i = index_restart[0]; index_i < upper_bound_[0];
				++index_i) {
			index_values_[0] = index_i;
			data_manager_.set_index_value(index_id_[0], index_i);

			for (int index_j = index_restart[1]; index_j < upper_bound_[1];
					++index_j) {
				index_values_[1] = index_j;
				data_manager_.set_index_value(index_id_[1], index_j);

				if (where_clause(1)) {
					for (int index_2 = index_restart[2];
							index_2 < upper_bound_[2]; ++index_2) {
						index_values_[2] = index_2;
						data_manager_.set_index_value(index_id_[2], index_2);

						if (where_clause(2)) {
							for (int index_3 = index_restart[3];
									index_3 < upper_bound_[3]; ++index_3) {
								index_values_[3] = index_3;
								data_manager_.set_index_value(index_id_[3],
										index_3);

								if (where_clause(3)) {
									iteration_++;
									if ((iteration_ - 1) % num_workers_
											== company_rank_) {
										return true;
									}
								} // where 3
							} // index_3
							for (int i = 3; i < num_indices_; ++i) {
								index_restart[i] = lower_seg_[i];
							}
						} // where 2
					} // index_2
					for (int i = 2; i < num_indices_; ++i) {
						index_restart[i] = lower_seg_[i];
					}
				} // where 1
			} // index_j
			for (int i = 1; i < num_indices_; ++i) {
				index_restart[i] = lower_seg_[i];
			}
		} // index_i
	}

	for (int i = 0; i < num_indices_; ++i) {
		index_restart[i] = index_values_[i];
	}

	for (int index_i = index_restart[0]; index_i < upper_bound_[0]; ++index_i) {
		index_values_[0] = index_i;
		data_manager_.set_index_value(index_id_[0], index_i);

		for (int index_j = index_restart[1]; index_j < upper_bound_[1];
				++index_j) {
			index_values_[1] = index_j;
			data_manager_.set_index_value(index_id_[1], index_j);

			if (where_clause(1)) {
				for (int index_2 = index_restart[2]; index_2 < upper_bound_[2];
						++index_2) {
					index_values_[2] = index_2;
					data_manager_.set_index_value(index_id_[2], index_2);

					if (where_clause(2)) {
						for (int index_3 = index_restart[3];
								index_3 < upper_bound_[3]; ++index_3) {
							index_values_[3] = index_3;
							data_manager_.set_index_value(index_id_[3],
									index_3);

							if (where_clause(3)) {
								if (loop_count > iteration_) {
									iteration_++;
									if ((iteration_ - 1) % num_workers_
											== company_rank_) {
										return true;
									}
								}
								++loop_count;
							} // where 3
						} // index_3
						for (int i = 3; i < num_indices_; ++i) {
							index_restart[i] = lower_seg_[i];
						}
					} // where 2
				} // index_2
				for (int i = 2; i < num_indices_; ++i) {
					index_restart[i] = lower_seg_[i];
				}
			} // where 1
		} // index_j
		for (int i = 1; i < num_indices_; ++i) {
			index_restart[i] = lower_seg_[i];
		}
	} // index_i

	return false; //this should be false here
}

Fragment_Nij_o_o_PardoLoopManager::Fragment_Nij_o_o_PardoLoopManager(
		int num_indices, const int (&index_id)[MAX_RANK],
		DataManager & data_manager, const SipTables & sip_tables,
		SIPMPIAttr & sip_mpi_attr, int num_where_clauses,
		Interpreter* interpreter, long& iteration) :
		FragmentPardoLoopManager(num_indices, index_id, data_manager,
				sip_tables), first_time_(true), iteration_(iteration), sip_mpi_attr_(
				sip_mpi_attr), num_where_clauses_(num_where_clauses), company_rank_(
				sip_mpi_attr.company_rank()), num_workers_(
				sip_mpi_attr_.num_workers()), interpreter_(interpreter) {

	form_elst_dist();
	//form_rcut_dist();
	//form_swao_frag();
	form_swocca_frag();
	//form_swvirta_frag();
}

Fragment_Nij_o_o_PardoLoopManager::~Fragment_Nij_o_o_PardoLoopManager() {
}

/*!
 -------------------------------------------
 _i_aa__
 -------------------------------------------

 PARDO ifrag, mu,nu "Frag{i}{aa}{}"
 where (int)SwAO_frag[(index)mu] == ifrag
 .
 .
 .
 */

/*
 for each new special fragment where clause pattern, this should be the only thing realy changed.
 see comment above for fragment_special_where_clause syntax
 */
bool Fragment_i_aa__PardoLoopManager::where_clause(int index) {
	bool where_;
	int ifrag = 0;
	int jfrag = 1;
	int ao = 1;
	int occ = 2;
	int virt = 3;
	int elst = 4;
	int rcut = 5;

	switch (index) {
	case 0:
		where_ = true;
		break;
	case 1:
	case 2:
	case 3:
	case 4:
	case 5:
		where_ = fragment_special_where_clause(ao, index, ifrag);
		break;
	default:
		where_ = false;
	}
	return where_;
}

bool Fragment_i_aa__PardoLoopManager::do_update() {
	if (to_exit_)
		return false;
	bool more_iters;
	bool where_clauses_value;

	interpreter_->skip_where_clauses(num_where_clauses_);

	if (first_time_) {
		first_time_ = false;
		more_iters = initialize_indices();

		where_clauses_value = true;
		for (int i = 1; i < num_indices_; ++i) {
			where_clauses_value = where_clauses_value && where_clause(i);
		}
		if (where_clauses_value) {
			iteration_++;
			if ((iteration_ - 1) % num_workers_ == company_rank_) {
				return true;
			}
		}
	}

	int index_restart[MAX_RANK];

	for (int i = 0; i < num_indices_; ++i) {
		index_restart[i] = index_values_[i];
	}

	int loop_count = iteration_;
	for (int index_i = index_restart[0]; index_i < upper_bound_[0]; ++index_i) {
		index_values_[0] = index_i;
		data_manager_.set_index_value(index_id_[0], index_i);

		for (int index_1 = index_restart[1]; index_1 < upper_bound_[1];
				++index_1) {
			index_values_[1] = index_1;
			data_manager_.set_index_value(index_id_[1], index_1);

			if (where_clause(1)) {
				for (int index_2 = index_restart[2]; index_2 < upper_bound_[2];
						++index_2) {
					index_values_[2] = index_2;
					data_manager_.set_index_value(index_id_[2], index_2);

					if (where_clause(2)) {
						if (loop_count > iteration_) {
							iteration_++;
							if ((iteration_ - 1) % num_workers_
									== company_rank_) {
								return true;
							}
						}
						++loop_count;
					} // where 2
				} // index_2
				for (int i = 2; i < num_indices_; ++i) {
					index_restart[i] = lower_seg_[i];
				}
			} // where 1
		} // index_1
		for (int i = 1; i < num_indices_; ++i) {
			index_restart[i] = lower_seg_[i];
		}
	} // index_i

	return false; //this should be false here
}

Fragment_i_aa__PardoLoopManager::Fragment_i_aa__PardoLoopManager(
		int num_indices, const int (&index_id)[MAX_RANK],
		DataManager & data_manager, const SipTables & sip_tables,
		SIPMPIAttr & sip_mpi_attr, int num_where_clauses,
		Interpreter* interpreter, long& iteration) :
		FragmentPardoLoopManager(num_indices, index_id, data_manager,
				sip_tables), first_time_(true), iteration_(iteration), sip_mpi_attr_(
				sip_mpi_attr), num_where_clauses_(num_where_clauses), company_rank_(
				sip_mpi_attr.company_rank()), num_workers_(
				sip_mpi_attr_.num_workers()), interpreter_(interpreter) {

	//form_elst_dist();
	//form_rcut_dist();
	form_swao_frag();
	//form_swocca_frag();
	//form_swvirta_frag();
}

Fragment_i_aa__PardoLoopManager::~Fragment_i_aa__PardoLoopManager() {}

/*!
 -------------------------------------------
 _ij_aaa__
 -------------------------------------------

 PARDO ifrag, jfrag, mu, i, b, j #GETLINE: Fragment_ij_aaa__
 where (int)elst_dist[ifrag,jfrag] == ifrag
 where (int)SwAO_frag[(index)mu] == ifrag
 */

/*
 for each new special fragment where clause pattern, this should be the only thing realy changed.
 see comment above for fragment_special_where_clause syntax
 */
bool Fragment_ij_aaa__PardoLoopManager::where_clause(int index) {
	bool where_;
	int ifrag = 0;
	int jfrag = 1;
	int ao = 1;
	int occ = 2;
	int virt = 3;
	int elst = 4;
	int rcut = 5;

	switch (index) {
	case 0:
	case 1:
		where_ = fragment_special_where_clause(elst, jfrag, ifrag);
		break;
	case 2:
	case 3:
	case 4:
	case 5:
		where_ = fragment_special_where_clause(ao, index, ifrag);
		break;
	default:
		where_ = false;
	}
	return where_;
}

bool Fragment_ij_aaa__PardoLoopManager::do_update() {
	if (to_exit_)
		return false;
	bool more_iters;
	bool where_clauses_value;

	interpreter_->skip_where_clauses(num_where_clauses_);

	if (first_time_) {
		first_time_ = false;
		more_iters = initialize_indices();

		where_clauses_value = true;
		for (int i = 1; i < num_indices_; ++i) {
			where_clauses_value = where_clauses_value && where_clause(i);
		}
		if (where_clauses_value) {
			iteration_++;
			if ((iteration_ - 1) % num_workers_ == company_rank_) {
				return true;
			}
		}
	}

	int index_restart[MAX_RANK];

	for (int i = 0; i < num_indices_; ++i) {
		index_restart[i] = index_values_[i];
	}

	int loop_count = iteration_;
	for (int index_i = index_restart[0]; index_i < upper_bound_[0]; ++index_i) {
		index_values_[0] = index_i;
		data_manager_.set_index_value(index_id_[0], index_i);

		for (int index_j = index_restart[1]; index_j < upper_bound_[1];
				++index_j) {
			index_values_[1] = index_j;
			data_manager_.set_index_value(index_id_[1], index_j);

			if (where_clause(1)) {
				for (int index_2 = index_restart[2]; index_2 < upper_bound_[2];
						++index_2) {
					index_values_[2] = index_2;
					data_manager_.set_index_value(index_id_[2], index_2);

					if (where_clause(2)) {
						for (int index_3 = index_restart[3];
								index_3 < upper_bound_[3]; ++index_3) {
							index_values_[3] = index_3;
							data_manager_.set_index_value(index_id_[3],
									index_3);

							if (where_clause(3)) {
								for (int index_4 = index_restart[4];
										index_4 < upper_bound_[4]; ++index_4) {
									index_values_[4] = index_4;
									data_manager_.set_index_value(index_id_[4],
											index_4);

									if (where_clause(4)) {
										if (loop_count > iteration_) {
											iteration_++;
											if ((iteration_ - 1) % num_workers_
													== company_rank_) {
												return true;
											}
										}
										++loop_count;
									} // where 4
								} // index_4
								for (int i = 4; i < num_indices_; ++i) {
									index_restart[i] = lower_seg_[i];
								}
							} // where 3
						} // index_3
						for (int i = 3; i < num_indices_; ++i) {
							index_restart[i] = lower_seg_[i];
						}
					} // where 2
				} // index_2
				for (int i = 2; i < num_indices_; ++i) {
					index_restart[i] = lower_seg_[i];
				}
			} // where 1
		} // index_j
		for (int i = 1; i < num_indices_; ++i) {
			index_restart[i] = lower_seg_[i];
		}
	} // index_i

	return false; //this should be false here
}

Fragment_ij_aaa__PardoLoopManager::Fragment_ij_aaa__PardoLoopManager(
		int num_indices, const int (&index_id)[MAX_RANK],
		DataManager & data_manager, const SipTables & sip_tables,
		SIPMPIAttr & sip_mpi_attr, int num_where_clauses,
		Interpreter* interpreter, long& iteration) :
		FragmentPardoLoopManager(num_indices, index_id, data_manager,
				sip_tables), first_time_(true), iteration_(iteration), sip_mpi_attr_(
				sip_mpi_attr), num_where_clauses_(num_where_clauses), company_rank_(
				sip_mpi_attr.company_rank()), num_workers_(
				sip_mpi_attr_.num_workers()), interpreter_(interpreter) {

	form_elst_dist();
	//form_rcut_dist();
	form_swao_frag();
	//form_swocca_frag();
	//form_swvirta_frag();
}

Fragment_ij_aaa__PardoLoopManager::~Fragment_ij_aaa__PardoLoopManager() {
}

/*!
 -------------------------------------------
 _ij_aa_a_
 -------------------------------------------
 */

/*
 for each new special fragment where clause pattern, this should be the only thing realy changed.
 see comment above for fragment_special_where_clause syntax
 */
bool Fragment_ij_aa_a_PardoLoopManager::where_clause(int index) {
	bool where_;
	int ifrag = 0;
	int jfrag = 1;
	int ao = 1;
	int occ = 2;
	int virt = 3;
	int elst = 4;
	int rcut = 5;

	switch (index) {
	case 0:
	case 1:
		where_ = fragment_special_where_clause(elst, jfrag, ifrag);
		break;
	case 2:
	case 3:
		where_ = fragment_special_where_clause(ao, index, ifrag);
		break;
	case 4:
	case 5:
		where_ = fragment_special_where_clause(ao, index, jfrag);
		break;
	default:
		where_ = false;
	}
	return where_;
}

bool Fragment_ij_aa_a_PardoLoopManager::do_update() {
	if (to_exit_)
		return false;
	bool more_iters;
	bool where_clauses_value;

	interpreter_->skip_where_clauses(num_where_clauses_);

	if (first_time_) {
		first_time_ = false;
		more_iters = initialize_indices();

		where_clauses_value = true;
		for (int i = 1; i < num_indices_; ++i) {
			where_clauses_value = where_clauses_value && where_clause(i);
		}
		if (where_clauses_value) {
			iteration_++;
			if ((iteration_ - 1) % num_workers_ == company_rank_) {
				return true;
			}
		}
	}

	int index_restart[MAX_RANK];

	for (int i = 0; i < num_indices_; ++i) {
		index_restart[i] = index_values_[i];
	}

	int loop_count = iteration_;
	for (int index_i = index_restart[0]; index_i < upper_bound_[0]; ++index_i) {
		index_values_[0] = index_i;
		data_manager_.set_index_value(index_id_[0], index_i);

		for (int index_j = index_restart[1]; index_j < upper_bound_[1];
				++index_j) {
			index_values_[1] = index_j;
			data_manager_.set_index_value(index_id_[1], index_j);

			if (where_clause(1)) {
				for (int index_2 = index_restart[2]; index_2 < upper_bound_[2];
						++index_2) {
					index_values_[2] = index_2;
					data_manager_.set_index_value(index_id_[2], index_2);

					if (where_clause(2)) {
						for (int index_3 = index_restart[3];
								index_3 < upper_bound_[3]; ++index_3) {
							index_values_[3] = index_3;
							data_manager_.set_index_value(index_id_[3],
									index_3);

							if (where_clause(3)) {
								for (int index_4 = index_restart[4];
										index_4 < upper_bound_[4]; ++index_4) {
									index_values_[4] = index_4;
									data_manager_.set_index_value(index_id_[4],
											index_4);

									if (where_clause(4)) {
										if (loop_count > iteration_) {
											iteration_++;
											if ((iteration_ - 1) % num_workers_
													== company_rank_) {
												return true;
											}
										}
										++loop_count;
									} // where 4
								} // index_4
								for (int i = 4; i < num_indices_; ++i) {
									index_restart[i] = lower_seg_[i];
								}
							} // where 3
						} // index_3
						for (int i = 3; i < num_indices_; ++i) {
							index_restart[i] = lower_seg_[i];
						}
					} // where 2
				} // index_2
				for (int i = 2; i < num_indices_; ++i) {
					index_restart[i] = lower_seg_[i];
				}
			} // where 1
		} // index_j
		for (int i = 1; i < num_indices_; ++i) {
			index_restart[i] = lower_seg_[i];
		}
	} // index_i

	return false; //this should be false here
}

Fragment_ij_aa_a_PardoLoopManager::Fragment_ij_aa_a_PardoLoopManager(
		int num_indices, const int (&index_id)[MAX_RANK],
		DataManager & data_manager, const SipTables & sip_tables,
		SIPMPIAttr & sip_mpi_attr, int num_where_clauses,
		Interpreter* interpreter, long& iteration) :
		FragmentPardoLoopManager(num_indices, index_id, data_manager,
				sip_tables), first_time_(true), iteration_(iteration), sip_mpi_attr_(
				sip_mpi_attr), num_where_clauses_(num_where_clauses), company_rank_(
				sip_mpi_attr.company_rank()), num_workers_(
				sip_mpi_attr_.num_workers()), interpreter_(interpreter) {

	form_elst_dist();
	//form_rcut_dist();
	form_swao_frag();
	//form_swocca_frag();
	//form_swvirta_frag();
}

Fragment_ij_aa_a_PardoLoopManager::~Fragment_ij_aa_a_PardoLoopManager() {
}

/*!
 -------------------------------------------
 _ij_ao_ao_
 -------------------------------------------
 */

/*
 for each new special fragment where clause pattern, this should be the only thing realy changed.
 see comment above for fragment_special_where_clause syntax
 */
bool Fragment_ij_ao_ao_PardoLoopManager::where_clause(int index) {
	bool where_;
	int ifrag = 0;
	int jfrag = 1;
	int ao = 1;
	int occ = 2;
	int virt = 3;
	int elst = 4;
	int rcut = 5;

	switch (index) {
	case 0:
	case 1:
		where_ = fragment_special_where_clause(elst, jfrag, ifrag);
		break;
	case 2:
		where_ = fragment_special_where_clause(ao, index, ifrag);
		break;
	case 3:
		where_ = fragment_special_where_clause(occ, index, ifrag);
		break;
	case 4:
		where_ = fragment_special_where_clause(ao, index, jfrag);
		break;
	case 5:
		where_ = fragment_special_where_clause(occ, index, jfrag);
		break;
	default:
		where_ = false;
	}
	return where_;
}

bool Fragment_ij_ao_ao_PardoLoopManager::do_update() {
	if (to_exit_)
		return false;
	bool more_iters;
	bool where_clauses_value;

	interpreter_->skip_where_clauses(num_where_clauses_);

	if (first_time_) {
		first_time_ = false;
		more_iters = initialize_indices();

		where_clauses_value = true;
		for (int i = 1; i < num_indices_; ++i) {
			where_clauses_value = where_clauses_value && where_clause(i);
		}
		if (where_clauses_value) {
			iteration_++;
			if ((iteration_ - 1) % num_workers_ == company_rank_) {
				return true;
			}
		}
	}

	int index_restart[MAX_RANK];

	for (int i = 0; i < num_indices_; ++i) {
		index_restart[i] = index_values_[i];
	}

	int loop_count = iteration_;
	for (int index_i = index_restart[0]; index_i < upper_bound_[0]; ++index_i) {
		index_values_[0] = index_i;
		data_manager_.set_index_value(index_id_[0], index_i);

		for (int index_j = index_restart[1]; index_j < upper_bound_[1];
				++index_j) {
			index_values_[1] = index_j;
			data_manager_.set_index_value(index_id_[1], index_j);

			if (where_clause(1)) {
				for (int index_2 = index_restart[2]; index_2 < upper_bound_[2];
						++index_2) {
					index_values_[2] = index_2;
					data_manager_.set_index_value(index_id_[2], index_2);

					if (where_clause(2)) {
						for (int index_3 = index_restart[3];
								index_3 < upper_bound_[3]; ++index_3) {
							index_values_[3] = index_3;
							data_manager_.set_index_value(index_id_[3],
									index_3);

							if (where_clause(3)) {
								for (int index_4 = index_restart[4];
										index_4 < upper_bound_[4]; ++index_4) {
									index_values_[4] = index_4;
									data_manager_.set_index_value(index_id_[4],
											index_4);

									if (where_clause(4)) {
										for (int index_5 = index_restart[5];
												index_5 < upper_bound_[5];
												++index_5) {
											index_values_[5] = index_5;
											data_manager_.set_index_value(
													index_id_[5], index_5);

											if (where_clause(5)) {
												if (loop_count > iteration_) {
													iteration_++;
													if ((iteration_ - 1)
															% num_workers_
															== company_rank_) {
														return true;
													}
												}
												++loop_count;
											} // where 5
										} // index_5
										for (int i = 5; i < num_indices_; ++i) {
											index_restart[i] = lower_seg_[i];
										}
									} // where 4
								} // index_4
								for (int i = 4; i < num_indices_; ++i) {
									index_restart[i] = lower_seg_[i];
								}
							} // where 3
						} // index_3
						for (int i = 3; i < num_indices_; ++i) {
							index_restart[i] = lower_seg_[i];
						}
					} // where 2
				} // index_2
				for (int i = 2; i < num_indices_; ++i) {
					index_restart[i] = lower_seg_[i];
				}
			} // where 1
		} // index_j
		for (int i = 1; i < num_indices_; ++i) {
			index_restart[i] = lower_seg_[i];
		}
	} // index_i

	return false; //this should be false here
}

Fragment_ij_ao_ao_PardoLoopManager::Fragment_ij_ao_ao_PardoLoopManager(
		int num_indices, const int (&index_id)[MAX_RANK],
		DataManager & data_manager, const SipTables & sip_tables,
		SIPMPIAttr & sip_mpi_attr, int num_where_clauses,
		Interpreter* interpreter, long& iteration) :
		FragmentPardoLoopManager(num_indices, index_id, data_manager,
				sip_tables), first_time_(true), iteration_(iteration), sip_mpi_attr_(
				sip_mpi_attr), num_where_clauses_(num_where_clauses), company_rank_(
				sip_mpi_attr.company_rank()), num_workers_(
				sip_mpi_attr_.num_workers()), interpreter_(interpreter) {

	form_elst_dist();
	//form_rcut_dist();
	form_swao_frag();
	form_swocca_frag();
	//form_swvirta_frag();
}

Fragment_ij_ao_ao_PardoLoopManager::~Fragment_ij_ao_ao_PardoLoopManager() {
}

/*!
 -------------------------------------------
 _ij_aa_oo_
 -------------------------------------------
 */

/*
 for each new special fragment where clause pattern, this should be the only thing realy changed.
 see comment above for fragment_special_where_clause syntax
 */
bool Fragment_ij_aa_oo_PardoLoopManager::where_clause(int index) {
	bool where_;
	int ifrag = 0;
	int jfrag = 1;
	int ao = 1;
	int occ = 2;
	int virt = 3;
	int elst = 4;
	int rcut = 5;

	switch (index) {
	case 0:
	case 1:
		where_ = fragment_special_where_clause(elst, jfrag, ifrag);
		break;
	case 2:
		where_ = fragment_special_where_clause(ao, index, ifrag);
		break;
	case 3:
		where_ = fragment_special_where_clause(ao, index, ifrag);
		break;
	case 4:
		where_ = fragment_special_where_clause(occ, index, jfrag);
		break;
	case 5:
		where_ = fragment_special_where_clause(occ, index, jfrag);
		break;
	default:
		where_ = false;
	}
	return where_;
}

bool Fragment_ij_aa_oo_PardoLoopManager::do_update() {
	if (to_exit_)
		return false;
	bool more_iters;
	bool where_clauses_value;

	interpreter_->skip_where_clauses(num_where_clauses_);

	if (first_time_) {
		first_time_ = false;
		more_iters = initialize_indices();

		where_clauses_value = true;
		for (int i = 1; i < num_indices_; ++i) {
			where_clauses_value = where_clauses_value && where_clause(i);
		}
		if (where_clauses_value) {
			iteration_++;
			if ((iteration_ - 1) % num_workers_ == company_rank_) {
				return true;
			}
		}
	}

	int index_restart[MAX_RANK];

	for (int i = 0; i < num_indices_; ++i) {
		index_restart[i] = index_values_[i];
	}

	int loop_count = iteration_;
	for (int index_i = index_restart[0]; index_i < upper_bound_[0]; ++index_i) {
		index_values_[0] = index_i;
		data_manager_.set_index_value(index_id_[0], index_i);

		for (int index_j = index_restart[1]; index_j < upper_bound_[1];
				++index_j) {
			index_values_[1] = index_j;
			data_manager_.set_index_value(index_id_[1], index_j);

			if (where_clause(1)) {
				for (int index_2 = index_restart[2]; index_2 < upper_bound_[2];
						++index_2) {
					index_values_[2] = index_2;
					data_manager_.set_index_value(index_id_[2], index_2);

					if (where_clause(2)) {
						for (int index_3 = index_restart[3];
								index_3 < upper_bound_[3]; ++index_3) {
							index_values_[3] = index_3;
							data_manager_.set_index_value(index_id_[3],
									index_3);

							if (where_clause(3)) {
								for (int index_4 = index_restart[4];
										index_4 < upper_bound_[4]; ++index_4) {
									index_values_[4] = index_4;
									data_manager_.set_index_value(index_id_[4],
											index_4);

									if (where_clause(4)) {
										for (int index_5 = index_restart[5];
												index_5 < upper_bound_[5];
												++index_5) {
											index_values_[5] = index_5;
											data_manager_.set_index_value(
													index_id_[5], index_5);

											if (where_clause(5)) {
												if (loop_count > iteration_) {
													iteration_++;
													if ((iteration_ - 1)
															% num_workers_
															== company_rank_) {
														return true;
													}
												}
												++loop_count;
											} // where 5
										} // index_5
										for (int i = 5; i < num_indices_; ++i) {
											index_restart[i] = lower_seg_[i];
										}
									} // where 4
								} // index_4
								for (int i = 4; i < num_indices_; ++i) {
									index_restart[i] = lower_seg_[i];
								}
							} // where 3
						} // index_3
						for (int i = 3; i < num_indices_; ++i) {
							index_restart[i] = lower_seg_[i];
						}
					} // where 2
				} // index_2
				for (int i = 2; i < num_indices_; ++i) {
					index_restart[i] = lower_seg_[i];
				}
			} // where 1
		} // index_j
		for (int i = 1; i < num_indices_; ++i) {
			index_restart[i] = lower_seg_[i];
		}
	} // index_i

	return false; //this should be false here
}

Fragment_ij_aa_oo_PardoLoopManager::Fragment_ij_aa_oo_PardoLoopManager(
		int num_indices, const int (&index_id)[MAX_RANK],
		DataManager & data_manager, const SipTables & sip_tables,
		SIPMPIAttr & sip_mpi_attr, int num_where_clauses,
		Interpreter* interpreter, long& iteration) :
		FragmentPardoLoopManager(num_indices, index_id, data_manager,
				sip_tables), first_time_(true), iteration_(iteration), sip_mpi_attr_(
				sip_mpi_attr), num_where_clauses_(num_where_clauses), company_rank_(
				sip_mpi_attr.company_rank()), num_workers_(
				sip_mpi_attr_.num_workers()), interpreter_(interpreter) {

	form_elst_dist();
	//form_rcut_dist();
	form_swao_frag();
	form_swocca_frag();
	//form_swvirta_frag();
}

Fragment_ij_aa_oo_PardoLoopManager::~Fragment_ij_aa_oo_PardoLoopManager() {
}


/*!
 -------------------------------------------
 _ij_aoa_o_
 -------------------------------------------
 */

/*
 for each new special fragment where clause pattern, this should be the only thing realy changed.
 see comment above for fragment_special_where_clause syntax
 */
bool Fragment_ij_aoa_o_PardoLoopManager::where_clause(int index) {
	bool where_;
	int ifrag = 0;
	int jfrag = 1;
	int ao = 1;
	int occ = 2;
	int virt = 3;
	int elst = 4;
	int rcut = 5;

	switch (index) {
	case 0:
	case 1:
		where_ = fragment_special_where_clause(elst, jfrag, ifrag);
		break;
	case 2:
		where_ = fragment_special_where_clause(ao, index, ifrag);
		break;
	case 3:
		where_ = fragment_special_where_clause(occ, index, ifrag);
		break;
	case 4:
		where_ = fragment_special_where_clause(ao, index, ifrag);
		break;
	case 5:
		where_ = fragment_special_where_clause(occ, index, jfrag);
		break;
	default:
		where_ = false;
	}
	return where_;
}

bool Fragment_ij_aoa_o_PardoLoopManager::do_update() {
	if (to_exit_)
		return false;
	bool more_iters;
	bool where_clauses_value;

	interpreter_->skip_where_clauses(num_where_clauses_);

	if (first_time_) {
		first_time_ = false;
		more_iters = initialize_indices();

		where_clauses_value = true;
		for (int i = 1; i < num_indices_; ++i) {
			where_clauses_value = where_clauses_value && where_clause(i);
		}
		if (where_clauses_value) {
			iteration_++;
			if ((iteration_ - 1) % num_workers_ == company_rank_) {
				return true;
			}
		}
	}

	int index_restart[MAX_RANK];

	for (int i = 0; i < num_indices_; ++i) {
		index_restart[i] = index_values_[i];
	}

	int loop_count = iteration_;
	for (int index_i = index_restart[0]; index_i < upper_bound_[0]; ++index_i) {
		index_values_[0] = index_i;
		data_manager_.set_index_value(index_id_[0], index_i);

		for (int index_j = index_restart[1]; index_j < upper_bound_[1];
				++index_j) {
			index_values_[1] = index_j;
			data_manager_.set_index_value(index_id_[1], index_j);

			if (where_clause(1)) {
				for (int index_2 = index_restart[2]; index_2 < upper_bound_[2];
						++index_2) {
					index_values_[2] = index_2;
					data_manager_.set_index_value(index_id_[2], index_2);

					if (where_clause(2)) {
						for (int index_3 = index_restart[3];
								index_3 < upper_bound_[3]; ++index_3) {
							index_values_[3] = index_3;
							data_manager_.set_index_value(index_id_[3],
									index_3);

							if (where_clause(3)) {
								for (int index_4 = index_restart[4];
										index_4 < upper_bound_[4]; ++index_4) {
									index_values_[4] = index_4;
									data_manager_.set_index_value(index_id_[4],
											index_4);

									if (where_clause(4)) {
										for (int index_5 = index_restart[5];
												index_5 < upper_bound_[5];
												++index_5) {
											index_values_[5] = index_5;
											data_manager_.set_index_value(
													index_id_[5], index_5);

											if (where_clause(5)) {
												if (loop_count > iteration_) {
													iteration_++;
													if ((iteration_ - 1)
															% num_workers_
															== company_rank_) {
														return true;
													}
												}
												++loop_count;
											} // where 5
										} // index_5
										for (int i = 5; i < num_indices_; ++i) {
											index_restart[i] = lower_seg_[i];
										}
									} // where 4
								} // index_4
								for (int i = 4; i < num_indices_; ++i) {
									index_restart[i] = lower_seg_[i];
								}
							} // where 3
						} // index_3
						for (int i = 3; i < num_indices_; ++i) {
							index_restart[i] = lower_seg_[i];
						}
					} // where 2
				} // index_2
				for (int i = 2; i < num_indices_; ++i) {
					index_restart[i] = lower_seg_[i];
				}
			} // where 1
		} // index_j
		for (int i = 1; i < num_indices_; ++i) {
			index_restart[i] = lower_seg_[i];
		}
	} // index_i

	return false; //this should be false here
}

Fragment_ij_aoa_o_PardoLoopManager::Fragment_ij_aoa_o_PardoLoopManager(
		int num_indices, const int (&index_id)[MAX_RANK],
		DataManager & data_manager, const SipTables & sip_tables,
		SIPMPIAttr & sip_mpi_attr, int num_where_clauses,
		Interpreter* interpreter, long& iteration) :
		FragmentPardoLoopManager(num_indices, index_id, data_manager,
				sip_tables), first_time_(true), iteration_(iteration), sip_mpi_attr_(
				sip_mpi_attr), num_where_clauses_(num_where_clauses), company_rank_(
				sip_mpi_attr.company_rank()), num_workers_(
				sip_mpi_attr_.num_workers()), interpreter_(interpreter) {

	form_elst_dist();
	//form_rcut_dist();
	form_swao_frag();
	form_swocca_frag();
	//form_swvirta_frag();
}

Fragment_ij_aoa_o_PardoLoopManager::~Fragment_ij_aoa_o_PardoLoopManager() {
}

/*!
 -------------------------------------------
 _ij_aoo_o_
 -------------------------------------------
 */

/*
 for each new special fragment where clause pattern, this should be the only thing realy changed.
 see comment above for fragment_special_where_clause syntax
 */
bool Fragment_ij_aoo_o_PardoLoopManager::where_clause(int index) {
	bool where_;
	int ifrag = 0;
	int jfrag = 1;
	int ao = 1;
	int occ = 2;
	int virt = 3;
	int elst = 4;
	int rcut = 5;

	switch (index) {
	case 0:
	case 1:
		where_ = fragment_special_where_clause(elst, jfrag, ifrag);
		break;
	case 2:
		where_ = fragment_special_where_clause(ao, index, ifrag);
		break;
	case 3:
		where_ = fragment_special_where_clause(occ, index, ifrag);
		break;
	case 4:
		where_ = fragment_special_where_clause(occ, index, ifrag);
		break;
	case 5:
		where_ = fragment_special_where_clause(occ, index, jfrag);
		break;
	default:
		where_ = false;
	}
	return where_;
}

bool Fragment_ij_aoo_o_PardoLoopManager::do_update() {
	if (to_exit_)
		return false;
	bool more_iters;
	bool where_clauses_value;

	interpreter_->skip_where_clauses(num_where_clauses_);

	if (first_time_) {
		first_time_ = false;
		more_iters = initialize_indices();

		where_clauses_value = true;
		for (int i = 1; i < num_indices_; ++i) {
			where_clauses_value = where_clauses_value && where_clause(i);
		}
		if (where_clauses_value) {
			iteration_++;
			if ((iteration_ - 1) % num_workers_ == company_rank_) {
				return true;
			}
		}
	}

	int index_restart[MAX_RANK];

	for (int i = 0; i < num_indices_; ++i) {
		index_restart[i] = index_values_[i];
	}

	int loop_count = iteration_;
	for (int index_i = index_restart[0]; index_i < upper_bound_[0]; ++index_i) {
		index_values_[0] = index_i;
		data_manager_.set_index_value(index_id_[0], index_i);

		for (int index_j = index_restart[1]; index_j < upper_bound_[1];
				++index_j) {
			index_values_[1] = index_j;
			data_manager_.set_index_value(index_id_[1], index_j);

			if (where_clause(1)) {
				for (int index_2 = index_restart[2]; index_2 < upper_bound_[2];
						++index_2) {
					index_values_[2] = index_2;
					data_manager_.set_index_value(index_id_[2], index_2);

					if (where_clause(2)) {
						for (int index_3 = index_restart[3];
								index_3 < upper_bound_[3]; ++index_3) {
							index_values_[3] = index_3;
							data_manager_.set_index_value(index_id_[3],
									index_3);

							if (where_clause(3)) {
								for (int index_4 = index_restart[4];
										index_4 < upper_bound_[4]; ++index_4) {
									index_values_[4] = index_4;
									data_manager_.set_index_value(index_id_[4],
											index_4);

									if (where_clause(4)) {
										for (int index_5 = index_restart[5];
												index_5 < upper_bound_[5];
												++index_5) {
											index_values_[5] = index_5;
											data_manager_.set_index_value(
													index_id_[5], index_5);

											if (where_clause(5)) {
												if (loop_count > iteration_) {
													iteration_++;
													if ((iteration_ - 1)
															% num_workers_
															== company_rank_) {
														return true;
													}
												}
												++loop_count;
											} // where 5
										} // index_5
										for (int i = 5; i < num_indices_; ++i) {
											index_restart[i] = lower_seg_[i];
										}
									} // where 4
								} // index_4
								for (int i = 4; i < num_indices_; ++i) {
									index_restart[i] = lower_seg_[i];
								}
							} // where 3
						} // index_3
						for (int i = 3; i < num_indices_; ++i) {
							index_restart[i] = lower_seg_[i];
						}
					} // where 2
				} // index_2
				for (int i = 2; i < num_indices_; ++i) {
					index_restart[i] = lower_seg_[i];
				}
			} // where 1
		} // index_j
		for (int i = 1; i < num_indices_; ++i) {
			index_restart[i] = lower_seg_[i];
		}
	} // index_i

	return false; //this should be false here
}

Fragment_ij_aoo_o_PardoLoopManager::Fragment_ij_aoo_o_PardoLoopManager(
		int num_indices, const int (&index_id)[MAX_RANK],
		DataManager & data_manager, const SipTables & sip_tables,
		SIPMPIAttr & sip_mpi_attr, int num_where_clauses,
		Interpreter* interpreter, long& iteration) :
		FragmentPardoLoopManager(num_indices, index_id, data_manager,
				sip_tables), first_time_(true), iteration_(iteration), sip_mpi_attr_(
				sip_mpi_attr), num_where_clauses_(num_where_clauses), company_rank_(
				sip_mpi_attr.company_rank()), num_workers_(
				sip_mpi_attr_.num_workers()), interpreter_(interpreter) {

	form_elst_dist();
	//form_rcut_dist();
	form_swao_frag();
	form_swocca_frag();
	//form_swvirta_frag();
}

Fragment_ij_aoo_o_PardoLoopManager::~Fragment_ij_aoo_o_PardoLoopManager() {
}





/*!
 -------------------------------------------
 _i_aaoo__
 -------------------------------------------
 */

/*
 for each new special fragment where clause pattern, this should be the only thing realy changed.
 see comment above for fragment_special_where_clause syntax
 */
bool Fragment_i_aaoo__PardoLoopManager::where_clause(int index) {
	bool where_;
	int ifrag = 0;
	int jfrag = 1;
	int ao = 1;
	int occ = 2;
	int virt = 3;
	int elst = 4;
	int rcut = 5;

	switch (index) {
	case 0:
		where_ = true;
		break;
	case 1:
		where_ = fragment_special_where_clause(ao, index, ifrag);
		break;
	case 2:
		where_ = fragment_special_where_clause(ao, index, ifrag);
		break;
	case 3:
		where_ = fragment_special_where_clause(occ, index, ifrag);
		break;
	case 4:
		where_ = fragment_special_where_clause(occ, index, ifrag);
		break;
	case 5:
		break;
	default:
		where_ = false;
	}
	return where_;
}

bool Fragment_i_aaoo__PardoLoopManager::do_update() {
	if (to_exit_)
		return false;
	bool more_iters;
	bool where_clauses_value;

	interpreter_->skip_where_clauses(num_where_clauses_);

	if (first_time_) {
		first_time_ = false;
		more_iters = initialize_indices();

		where_clauses_value = true;
		for (int i = 1; i < num_indices_; ++i) {
			where_clauses_value = where_clauses_value && where_clause(i);
		}
		if (where_clauses_value) {
			iteration_++;
			if ((iteration_ - 1) % num_workers_ == company_rank_) {
				return true;
			}
		}
	}

	int index_restart[MAX_RANK];

	for (int i = 0; i < num_indices_; ++i) {
		index_restart[i] = index_values_[i];
	}

	int loop_count = iteration_;
	for (int index_i = index_restart[0]; index_i < upper_bound_[0]; ++index_i) {
		index_values_[0] = index_i;
		data_manager_.set_index_value(index_id_[0], index_i);

		for (int index_1 = index_restart[1]; index_1 < upper_bound_[1];
				++index_1) {
			index_values_[1] = index_1;
			data_manager_.set_index_value(index_id_[1], index_1);

			if (where_clause(1)) {
				for (int index_2 = index_restart[2]; index_2 < upper_bound_[2];
						++index_2) {
					index_values_[2] = index_2;
					data_manager_.set_index_value(index_id_[2], index_2);

					if (where_clause(2)) {
						for (int index_3 = index_restart[3];
								index_3 < upper_bound_[3]; ++index_3) {
							index_values_[3] = index_3;
							data_manager_.set_index_value(index_id_[3],
									index_3);

							if (where_clause(3)) {
								for (int index_4 = index_restart[4];
										index_4 < upper_bound_[4]; ++index_4) {
									index_values_[4] = index_4;
									data_manager_.set_index_value(index_id_[4],
											index_4);

									if (where_clause(4)) {
										if (loop_count > iteration_) {
											iteration_++;
											if ((iteration_ - 1) % num_workers_
													== company_rank_) {
												return true;
											}
										}
										++loop_count;
									} // where 4
								} // index_4
								for (int i = 4; i < num_indices_; ++i) {
									index_restart[i] = lower_seg_[i];
								}
							} // where 3
						} // index_3
						for (int i = 3; i < num_indices_; ++i) {
							index_restart[i] = lower_seg_[i];
						}
					} // where 2
				} // index_2
				for (int i = 2; i < num_indices_; ++i) {
					index_restart[i] = lower_seg_[i];
				}
			} // where 1
		} // index_1
		for (int i = 1; i < num_indices_; ++i) {
			index_restart[i] = lower_seg_[i];
		}
	} // index_i

	return false; //this should be false here
}

Fragment_i_aaoo__PardoLoopManager::Fragment_i_aaoo__PardoLoopManager(
		int num_indices, const int (&index_id)[MAX_RANK],
		DataManager & data_manager, const SipTables & sip_tables,
		SIPMPIAttr & sip_mpi_attr, int num_where_clauses,
		Interpreter* interpreter, long& iteration) :
		FragmentPardoLoopManager(num_indices, index_id, data_manager,
				sip_tables), first_time_(true), iteration_(iteration), sip_mpi_attr_(
				sip_mpi_attr), num_where_clauses_(num_where_clauses), company_rank_(
				sip_mpi_attr.company_rank()), num_workers_(
				sip_mpi_attr_.num_workers()), interpreter_(interpreter) {


	//form_elst_dist();
	//form_rcut_dist();
	form_swao_frag();
	form_swocca_frag();
	//form_swvirta_frag();
}

Fragment_i_aaoo__PardoLoopManager::~Fragment_i_aaoo__PardoLoopManager() {
}

/*!
 -------------------------------------------
 _i_aovo__
 -------------------------------------------
 */

/*
 for each new special fragment where clause pattern, this should be the only thing realy changed.
 see comment above for fragment_special_where_clause syntax
 */
bool Fragment_i_aovo__PardoLoopManager::where_clause(int index) {
	bool where_;
	int ifrag = 0;
	int jfrag = 1;
	int ao = 1;
	int occ = 2;
	int virt = 3;
	int elst = 4;
	int rcut = 5;

	switch (index) {
	case 0:
		where_ = true;
		break;
	case 1:
		where_ = fragment_special_where_clause(ao, index, ifrag);
		break;
	case 2:
		where_ = fragment_special_where_clause(occ, index, ifrag);
		break;
	case 3:
		where_ = fragment_special_where_clause(virt, index, ifrag);
		break;
	case 4:
		where_ = fragment_special_where_clause(occ, index, ifrag);
		break;
	case 5:
		break;
	default:
		where_ = false;
	}
	return where_;
}

bool Fragment_i_aovo__PardoLoopManager::do_update() {
	if (to_exit_)
		return false;
	bool more_iters;
	bool where_clauses_value;

	interpreter_->skip_where_clauses(num_where_clauses_);

	if (first_time_) {
		first_time_ = false;
		more_iters = initialize_indices();

		where_clauses_value = true;
		for (int i = 1; i < num_indices_; ++i) {
			where_clauses_value = where_clauses_value && where_clause(i);
		}
		if (where_clauses_value) {
			iteration_++;
			if ((iteration_ - 1) % num_workers_ == company_rank_) {
				return true;
			}
		}
	}

	int index_restart[MAX_RANK];

	for (int i = 0; i < num_indices_; ++i) {
		index_restart[i] = index_values_[i];
	}

	int loop_count = iteration_;
	for (int index_i = index_restart[0]; index_i < upper_bound_[0]; ++index_i) {
		index_values_[0] = index_i;
		data_manager_.set_index_value(index_id_[0], index_i);

		for (int index_1 = index_restart[1]; index_1 < upper_bound_[1];
				++index_1) {
			index_values_[1] = index_1;
			data_manager_.set_index_value(index_id_[1], index_1);

			if (where_clause(1)) {
				for (int index_2 = index_restart[2]; index_2 < upper_bound_[2];
						++index_2) {
					index_values_[2] = index_2;
					data_manager_.set_index_value(index_id_[2], index_2);

					if (where_clause(2)) {
						for (int index_3 = index_restart[3];
								index_3 < upper_bound_[3]; ++index_3) {
							index_values_[3] = index_3;
							data_manager_.set_index_value(index_id_[3],
									index_3);

							if (where_clause(3)) {
								for (int index_4 = index_restart[4];
										index_4 < upper_bound_[4]; ++index_4) {
									index_values_[4] = index_4;
									data_manager_.set_index_value(index_id_[4],
											index_4);

									if (where_clause(4)) {
										if (loop_count > iteration_) {
											iteration_++;
											if ((iteration_ - 1) % num_workers_
													== company_rank_) {
												return true;
											}
										}
										++loop_count;
									} // where 4
								} // index_4
								for (int i = 4; i < num_indices_; ++i) {
									index_restart[i] = lower_seg_[i];
								}
							} // where 3
						} // index_3
						for (int i = 3; i < num_indices_; ++i) {
							index_restart[i] = lower_seg_[i];
						}
					} // where 2
				} // index_2
				for (int i = 2; i < num_indices_; ++i) {
					index_restart[i] = lower_seg_[i];
				}
			} // where 1
		} // index_1
		for (int i = 1; i < num_indices_; ++i) {
			index_restart[i] = lower_seg_[i];
		}
	} // index_i

	return false; //this should be false here
}

Fragment_i_aovo__PardoLoopManager::Fragment_i_aovo__PardoLoopManager(
		int num_indices, const int (&index_id)[MAX_RANK],
		DataManager & data_manager, const SipTables & sip_tables,
		SIPMPIAttr & sip_mpi_attr, int num_where_clauses,
		Interpreter* interpreter, long& iteration) :
		FragmentPardoLoopManager(num_indices, index_id, data_manager,
				sip_tables), first_time_(true), iteration_(iteration), sip_mpi_attr_(
				sip_mpi_attr), num_where_clauses_(num_where_clauses), company_rank_(
				sip_mpi_attr.company_rank()), num_workers_(
				sip_mpi_attr_.num_workers()), interpreter_(interpreter) {

	//form_elst_dist();
	//form_rcut_dist();
	form_swao_frag();
	form_swocca_frag();
	form_swvirta_frag();
}

Fragment_i_aovo__PardoLoopManager::~Fragment_i_aovo__PardoLoopManager() {
}

/*!
 -------------------------------------------
 _i_aaaa__
 -------------------------------------------
 */

/*
 for each new special fragment where clause pattern, this should be the only thing realy changed.
 see comment above for fragment_special_where_clause syntax
 */
bool Fragment_i_aaaa__PardoLoopManager::where_clause(int index) {
	bool where_;
	int ifrag = 0;
	int jfrag = 1;
	int ao = 1;
	int occ = 2;
	int virt = 3;
	int elst = 4;
	int rcut = 5;

	switch (index) {
	case 0:
		where_ = true;
		break;
	case 1:
		where_ = fragment_special_where_clause(ao, index, ifrag);
		break;
	case 2:
		where_ = fragment_special_where_clause(ao, index, ifrag);
		break;
	case 3:
		where_ = fragment_special_where_clause(ao, index, ifrag);
		break;
	case 4:
		where_ = fragment_special_where_clause(ao, index, ifrag);
		break;
	case 5:
		break;
	default:
		where_ = false;
	}
	return where_;
}

bool Fragment_i_aaaa__PardoLoopManager::do_update() {
	if (to_exit_)
		return false;
	bool more_iters;
	bool where_clauses_value;

	interpreter_->skip_where_clauses(num_where_clauses_);

	if (first_time_) {
		first_time_ = false;
		more_iters = initialize_indices();

		where_clauses_value = true;
		for (int i = 1; i < num_indices_; ++i) {
			where_clauses_value = where_clauses_value && where_clause(i);
		}
		if (where_clauses_value) {
			iteration_++;
			if ((iteration_ - 1) % num_workers_ == company_rank_) {
				return true;
			}
		}
	}

	int index_restart[MAX_RANK];

	for (int i = 0; i < num_indices_; ++i) {
		index_restart[i] = index_values_[i];
	}

	int loop_count = iteration_;
	for (int index_i = index_restart[0]; index_i < upper_bound_[0]; ++index_i) {
		index_values_[0] = index_i;
		data_manager_.set_index_value(index_id_[0], index_i);

		for (int index_1 = index_restart[1]; index_1 < upper_bound_[1];
				++index_1) {
			index_values_[1] = index_1;
			data_manager_.set_index_value(index_id_[1], index_1);

			if (where_clause(1)) {
				for (int index_2 = index_restart[2]; index_2 < upper_bound_[2];
						++index_2) {
					index_values_[2] = index_2;
					data_manager_.set_index_value(index_id_[2], index_2);

					if (where_clause(2)) {
						for (int index_3 = index_restart[3];
								index_3 < upper_bound_[3]; ++index_3) {
							index_values_[3] = index_3;
							data_manager_.set_index_value(index_id_[3],
									index_3);

							if (where_clause(3)) {
								for (int index_4 = index_restart[4];
										index_4 < upper_bound_[4]; ++index_4) {
									index_values_[4] = index_4;
									data_manager_.set_index_value(index_id_[4],
											index_4);

									if (where_clause(4)) {
										if (loop_count > iteration_) {
											iteration_++;
											if ((iteration_ - 1) % num_workers_
													== company_rank_) {
												return true;
											}
										}
										++loop_count;
									} // where 4
								} // index_4
								for (int i = 4; i < num_indices_; ++i) {
									index_restart[i] = lower_seg_[i];
								}
							} // where 3
						} // index_3
						for (int i = 3; i < num_indices_; ++i) {
							index_restart[i] = lower_seg_[i];
						}
					} // where 2
				} // index_2
				for (int i = 2; i < num_indices_; ++i) {
					index_restart[i] = lower_seg_[i];
				}
			} // where 1
		} // index_1
		for (int i = 1; i < num_indices_; ++i) {
			index_restart[i] = lower_seg_[i];
		}
	} // index_i

	return false; //this should be false here
}

Fragment_i_aaaa__PardoLoopManager::Fragment_i_aaaa__PardoLoopManager(
		int num_indices, const int (&index_id)[MAX_RANK],
		DataManager & data_manager, const SipTables & sip_tables,
		SIPMPIAttr & sip_mpi_attr, int num_where_clauses,
		Interpreter* interpreter, long& iteration) :
		FragmentPardoLoopManager(num_indices, index_id, data_manager,
				sip_tables), first_time_(true), iteration_(iteration), sip_mpi_attr_(
				sip_mpi_attr), num_where_clauses_(num_where_clauses), company_rank_(
				sip_mpi_attr.company_rank()), num_workers_(
				sip_mpi_attr_.num_workers()), interpreter_(interpreter) {

	//form_elst_dist();
	//form_rcut_dist();
	form_swao_frag();
	//form_swocca_frag();
	//form_swvirta_frag();
}

Fragment_i_aaaa__PardoLoopManager::~Fragment_i_aaaa__PardoLoopManager() {
}

/*!
 -------------------------------------------
 _i_aoo__
 -------------------------------------------
 */

/*
 for each new special fragment where clause pattern, this should be the only thing realy changed.
 see comment above for fragment_special_where_clause syntax
 */
bool Fragment_i_aoo__PardoLoopManager::where_clause(int index) {
	bool where_;
	int ifrag = 0;
	int jfrag = 1;
	int ao = 1;
	int occ = 2;
	int virt = 3;
	int elst = 4;
	int rcut = 5;

	switch (index) {
	case 0:
		where_ = true;
		break;
	case 1:
		where_ = fragment_special_where_clause(ao, index, ifrag);
		break;
	case 2:
		where_ = fragment_special_where_clause(occ, index, ifrag);
		break;
	case 3:
		where_ = fragment_special_where_clause(occ, index, ifrag);
		break;
	case 4:
		break;
	case 5:
		break;
	default:
		where_ = false;
	}
	return where_;
}

bool Fragment_i_aoo__PardoLoopManager::do_update() {
	if (to_exit_)
		return false;
	bool more_iters;
	bool where_clauses_value;

	interpreter_->skip_where_clauses(num_where_clauses_);

	if (first_time_) {
		first_time_ = false;
		more_iters = initialize_indices();

		where_clauses_value = true;
		for (int i = 1; i < num_indices_; ++i) {
			where_clauses_value = where_clauses_value && where_clause(i);
		}
		if (where_clauses_value) {
			iteration_++;
			if ((iteration_ - 1) % num_workers_ == company_rank_) {
				return true;
			}
		}
	}

	int index_restart[MAX_RANK];

	for (int i = 0; i < num_indices_; ++i) {
		index_restart[i] = index_values_[i];
	}

	int loop_count = iteration_;
	for (int index_i = index_restart[0]; index_i < upper_bound_[0]; ++index_i) {
		index_values_[0] = index_i;
		data_manager_.set_index_value(index_id_[0], index_i);

		for (int index_1 = index_restart[1]; index_1 < upper_bound_[1];
				++index_1) {
			index_values_[1] = index_1;
			data_manager_.set_index_value(index_id_[1], index_1);

			if (where_clause(1)) {
				for (int index_2 = index_restart[2]; index_2 < upper_bound_[2];
						++index_2) {
					index_values_[2] = index_2;
					data_manager_.set_index_value(index_id_[2], index_2);

					if (where_clause(2)) {
						for (int index_3 = index_restart[3];
								index_3 < upper_bound_[3]; ++index_3) {
							index_values_[3] = index_3;
							data_manager_.set_index_value(index_id_[3],
									index_3);

							if (where_clause(3)) {
								if (loop_count > iteration_) {
									iteration_++;
									if ((iteration_ - 1) % num_workers_
											== company_rank_) {
										return true;
									}
								}
								++loop_count;
							} // where 3
						} // index_3
						for (int i = 3; i < num_indices_; ++i) {
							index_restart[i] = lower_seg_[i];
						}
					} // where 2
				} // index_2
				for (int i = 2; i < num_indices_; ++i) {
					index_restart[i] = lower_seg_[i];
				}
			} // where 1
		} // index_1
		for (int i = 1; i < num_indices_; ++i) {
			index_restart[i] = lower_seg_[i];
		}
	} // index_i

	return false; //this should be false here
}

Fragment_i_aoo__PardoLoopManager::Fragment_i_aoo__PardoLoopManager(
		int num_indices, const int (&index_id)[MAX_RANK],
		DataManager & data_manager, const SipTables & sip_tables,
		SIPMPIAttr & sip_mpi_attr, int num_where_clauses,
		Interpreter* interpreter, long& iteration) :
		FragmentPardoLoopManager(num_indices, index_id, data_manager,
				sip_tables), first_time_(true), iteration_(iteration), sip_mpi_attr_(
				sip_mpi_attr), num_where_clauses_(num_where_clauses), company_rank_(
				sip_mpi_attr.company_rank()), num_workers_(
				sip_mpi_attr_.num_workers()), interpreter_(interpreter) {

	//form_elst_dist();
	//form_rcut_dist();
	form_swao_frag();
	form_swocca_frag();
	//form_swvirta_frag();
}

Fragment_i_aoo__PardoLoopManager::~Fragment_i_aoo__PardoLoopManager() {
}

/*!
 -------------------------------------------
 _NRij_vovo__
 -------------------------------------------
 */

/*
 for each new special fragment where clause pattern, this should be the only thing realy changed.
 see comment above for fragment_special_where_clause syntax
 */
bool Fragment_NRij_vovo__PardoLoopManager::where_clause(int index) {
	bool where_;
	int ifrag = 0;
	int jfrag = 1;
	int ao = 1;
	int occ = 2;
	int virt = 3;
	int elst = 4;
	int rcut = 5;
	int NE = 0;

	switch (index) {
	case 0:
	case 1:
		where_ = fragment_special_where_clause(rcut, jfrag, ifrag)
				&& fragment_special_where_clause(NE, jfrag, ifrag);
		break;
	case 2:
		where_ = fragment_special_where_clause(virt, index, ifrag);
		break;
	case 3:
		where_ = fragment_special_where_clause(occ, index, ifrag);
		break;
	case 4:
		where_ = fragment_special_where_clause(virt, index, ifrag);
		break;
	case 5:
		where_ = fragment_special_where_clause(occ, index, ifrag);
		break;
	default:
		where_ = false;
	}
	return where_;
}

bool Fragment_NRij_vovo__PardoLoopManager::do_update() {
	if (to_exit_)
		return false;
	bool more_iters;
	bool where_clauses_value;

	interpreter_->skip_where_clauses(num_where_clauses_);

	int loop_count = iteration_;
	int index_restart[MAX_RANK];

	if (first_time_) {
		first_time_ = false;
		more_iters = initialize_indices();

		for (int i = 0; i < num_indices_; ++i) {
			index_restart[i] = index_values_[i];
		}

		for (int index_i = index_restart[0]; index_i < upper_bound_[0];
				++index_i) {
			index_values_[0] = index_i;
			data_manager_.set_index_value(index_id_[0], index_i);

			for (int index_j = index_restart[1]; index_j < upper_bound_[1];
					++index_j) {
				index_values_[1] = index_j;
				data_manager_.set_index_value(index_id_[1], index_j);

				if (where_clause(1)) {
					for (int index_2 = index_restart[2];
							index_2 < upper_bound_[2]; ++index_2) {
						index_values_[2] = index_2;
						data_manager_.set_index_value(index_id_[2], index_2);

						if (where_clause(2)) {
							for (int index_3 = index_restart[3];
									index_3 < upper_bound_[3]; ++index_3) {
								index_values_[3] = index_3;
								data_manager_.set_index_value(index_id_[3],
										index_3);

								if (where_clause(3)) {
									for (int index_4 = index_restart[4];
											index_4 < upper_bound_[4];
											++index_4) {
										index_values_[4] = index_4;
										data_manager_.set_index_value(
												index_id_[4], index_4);

										if (where_clause(4)) {
											for (int index_5 = index_restart[5];
													index_5 < upper_bound_[5];
													++index_5) {
												index_values_[5] = index_5;
												data_manager_.set_index_value(
														index_id_[5], index_5);

												if (where_clause(5)) {
													iteration_++;
													if ((iteration_ - 1)
															% num_workers_
															== company_rank_) {
														return true;
													}
												} // where 5
											} // index_5
											for (int i = 5; i < num_indices_;
													++i) {
												index_restart[i] =
														lower_seg_[i];
											}
										} // where 4
									} // index_4
									for (int i = 4; i < num_indices_; ++i) {
										index_restart[i] = lower_seg_[i];
									}
								} // where 3
							} // index_3
							for (int i = 3; i < num_indices_; ++i) {
								index_restart[i] = lower_seg_[i];
							}
						} // where 2
					} // index_2
					for (int i = 2; i < num_indices_; ++i) {
						index_restart[i] = lower_seg_[i];
					}
				} // where 1
			} // index_j
			for (int i = 1; i < num_indices_; ++i) {
				index_restart[i] = lower_seg_[i];
			}
		} // index_i
	}

	for (int i = 0; i < num_indices_; ++i) {
		index_restart[i] = index_values_[i];
	}

	for (int index_i = index_restart[0]; index_i < upper_bound_[0]; ++index_i) {
		index_values_[0] = index_i;
		data_manager_.set_index_value(index_id_[0], index_i);

		for (int index_j = index_restart[1]; index_j < upper_bound_[1];
				++index_j) {
			index_values_[1] = index_j;
			data_manager_.set_index_value(index_id_[1], index_j);

			if (where_clause(1)) {
				for (int index_2 = index_restart[2]; index_2 < upper_bound_[2];
						++index_2) {
					index_values_[2] = index_2;
					data_manager_.set_index_value(index_id_[2], index_2);

					if (where_clause(2)) {
						for (int index_3 = index_restart[3];
								index_3 < upper_bound_[3]; ++index_3) {
							index_values_[3] = index_3;
							data_manager_.set_index_value(index_id_[3],
									index_3);

							if (where_clause(3)) {
								for (int index_4 = index_restart[4];
										index_4 < upper_bound_[4]; ++index_4) {
									index_values_[4] = index_4;
									data_manager_.set_index_value(index_id_[4],
											index_4);

									if (where_clause(4)) {
										for (int index_5 = index_restart[5];
												index_5 < upper_bound_[5];
												++index_5) {
											index_values_[5] = index_5;
											data_manager_.set_index_value(
													index_id_[5], index_5);

											if (where_clause(5)) {
												if (loop_count > iteration_) {
													iteration_++;
													if ((iteration_ - 1)
															% num_workers_
															== company_rank_) {
														return true;
													}
												}
												++loop_count;
											} // where 5
										} // index_5
										for (int i = 5; i < num_indices_; ++i) {
											index_restart[i] = lower_seg_[i];
										}
									} // where 4
								} // index_4
								for (int i = 4; i < num_indices_; ++i) {
									index_restart[i] = lower_seg_[i];
								}
							} // where 3
						} // index_3
						for (int i = 3; i < num_indices_; ++i) {
							index_restart[i] = lower_seg_[i];
						}
					} // where 2
				} // index_2
				for (int i = 2; i < num_indices_; ++i) {
					index_restart[i] = lower_seg_[i];
				}
			} // where 1
		} // index_j
		for (int i = 1; i < num_indices_; ++i) {
			index_restart[i] = lower_seg_[i];
		}
	} // index_i

	return false; //this should be false here
}

Fragment_NRij_vovo__PardoLoopManager::Fragment_NRij_vovo__PardoLoopManager(
		int num_indices, const int (&index_id)[MAX_RANK],
		DataManager & data_manager, const SipTables & sip_tables,
		SIPMPIAttr & sip_mpi_attr, int num_where_clauses,
		Interpreter* interpreter, long& iteration) :
		FragmentPardoLoopManager(num_indices, index_id, data_manager,
				sip_tables), first_time_(true), iteration_(iteration), sip_mpi_attr_(
				sip_mpi_attr), num_where_clauses_(num_where_clauses), company_rank_(
				sip_mpi_attr.company_rank()), num_workers_(
				sip_mpi_attr_.num_workers()), interpreter_(interpreter) {

	//form_elst_dist();
	form_rcut_dist();
	//form_swao_frag();
	form_swocca_frag();
	form_swvirta_frag();
}

Fragment_NRij_vovo__PardoLoopManager::~Fragment_NRij_vovo__PardoLoopManager() {
}

/*!
 -------------------------------------------
 _NRij_ao_ao_
 -------------------------------------------
 */

/*
 for each new special fragment where clause pattern, this should be the only thing realy changed.
 see comment above for fragment_special_where_clause syntax
 */
bool Fragment_NRij_ao_ao_PardoLoopManager::where_clause(int index) {
	bool where_;
	int ifrag = 0;
	int jfrag = 1;
	int ao = 1;
	int occ = 2;
	int virt = 3;
	int elst = 4;
	int rcut = 5;
	int NE = 0;

	switch (index) {
	case 0:
	case 1:
		where_ = fragment_special_where_clause(rcut, jfrag, ifrag)
				&& fragment_special_where_clause(NE, jfrag, ifrag);
		break;
	case 2:
		where_ = fragment_special_where_clause(ao, index, ifrag);
		break;
	case 3:
		where_ = fragment_special_where_clause(occ, index, ifrag);
		break;
	case 4:
		where_ = fragment_special_where_clause(ao, index, jfrag);
		break;
	case 5:
		where_ = fragment_special_where_clause(occ, index, jfrag);
		break;
	default:
		where_ = false;
	}
	return where_;
}

bool Fragment_NRij_ao_ao_PardoLoopManager::do_update() {
	if (to_exit_)
		return false;
	bool more_iters;
	bool where_clauses_value;

	interpreter_->skip_where_clauses(num_where_clauses_);

	int loop_count = iteration_;
	int index_restart[MAX_RANK];

	if (first_time_) {
		first_time_ = false;
		more_iters = initialize_indices();

		for (int i = 0; i < num_indices_; ++i) {
			index_restart[i] = index_values_[i];
		}

		for (int index_i = index_restart[0]; index_i < upper_bound_[0];
				++index_i) {
			index_values_[0] = index_i;
			data_manager_.set_index_value(index_id_[0], index_i);

			for (int index_j = index_restart[1]; index_j < upper_bound_[1];
					++index_j) {
				index_values_[1] = index_j;
				data_manager_.set_index_value(index_id_[1], index_j);

				if (where_clause(1)) {
					for (int index_2 = index_restart[2];
							index_2 < upper_bound_[2]; ++index_2) {
						index_values_[2] = index_2;
						data_manager_.set_index_value(index_id_[2], index_2);

						if (where_clause(2)) {
							for (int index_3 = index_restart[3];
									index_3 < upper_bound_[3]; ++index_3) {
								index_values_[3] = index_3;
								data_manager_.set_index_value(index_id_[3],
										index_3);

								if (where_clause(3)) {
									for (int index_4 = index_restart[4];
											index_4 < upper_bound_[4];
											++index_4) {
										index_values_[4] = index_4;
										data_manager_.set_index_value(
												index_id_[4], index_4);

										if (where_clause(4)) {
											for (int index_5 = index_restart[5];
													index_5 < upper_bound_[5];
													++index_5) {
												index_values_[5] = index_5;
												data_manager_.set_index_value(
														index_id_[5], index_5);

												if (where_clause(5)) {
													iteration_++;
													if ((iteration_ - 1)
															% num_workers_
															== company_rank_) {
														return true;
													}
												} // where 5
											} // index_5
											for (int i = 5; i < num_indices_;
													++i) {
												index_restart[i] =
														lower_seg_[i];
											}
										} // where 4
									} // index_4
									for (int i = 4; i < num_indices_; ++i) {
										index_restart[i] = lower_seg_[i];
									}
								} // where 3
							} // index_3
							for (int i = 3; i < num_indices_; ++i) {
								index_restart[i] = lower_seg_[i];
							}
						} // where 2
					} // index_2
					for (int i = 2; i < num_indices_; ++i) {
						index_restart[i] = lower_seg_[i];
					}
				} // where 1
			} // index_j
			for (int i = 1; i < num_indices_; ++i) {
				index_restart[i] = lower_seg_[i];
			}
		} // index_i
	}

	for (int i = 0; i < num_indices_; ++i) {
		index_restart[i] = index_values_[i];
	}

	for (int index_i = index_restart[0]; index_i < upper_bound_[0]; ++index_i) {
		index_values_[0] = index_i;
		data_manager_.set_index_value(index_id_[0], index_i);

		for (int index_j = index_restart[1]; index_j < upper_bound_[1];
				++index_j) {
			index_values_[1] = index_j;
			data_manager_.set_index_value(index_id_[1], index_j);

			if (where_clause(1)) {
				for (int index_2 = index_restart[2]; index_2 < upper_bound_[2];
						++index_2) {
					index_values_[2] = index_2;
					data_manager_.set_index_value(index_id_[2], index_2);

					if (where_clause(2)) {
						for (int index_3 = index_restart[3];
								index_3 < upper_bound_[3]; ++index_3) {
							index_values_[3] = index_3;
							data_manager_.set_index_value(index_id_[3],
									index_3);

							if (where_clause(3)) {
								for (int index_4 = index_restart[4];
										index_4 < upper_bound_[4]; ++index_4) {
									index_values_[4] = index_4;
									data_manager_.set_index_value(index_id_[4],
											index_4);

									if (where_clause(4)) {
										for (int index_5 = index_restart[5];
												index_5 < upper_bound_[5];
												++index_5) {
											index_values_[5] = index_5;
											data_manager_.set_index_value(
													index_id_[5], index_5);

											if (where_clause(5)) {
												if (loop_count > iteration_) {
													iteration_++;
													if ((iteration_ - 1)
															% num_workers_
															== company_rank_) {
														return true;
													}
												}
												++loop_count;
											} // where 5
										} // index_5
										for (int i = 5; i < num_indices_; ++i) {
											index_restart[i] = lower_seg_[i];
										}
									} // where 4
								} // index_4
								for (int i = 4; i < num_indices_; ++i) {
									index_restart[i] = lower_seg_[i];
								}
							} // where 3
						} // index_3
						for (int i = 3; i < num_indices_; ++i) {
							index_restart[i] = lower_seg_[i];
						}
					} // where 2
				} // index_2
				for (int i = 2; i < num_indices_; ++i) {
					index_restart[i] = lower_seg_[i];
				}
			} // where 1
		} // index_j
		for (int i = 1; i < num_indices_; ++i) {
			index_restart[i] = lower_seg_[i];
		}
	} // index_i

	return false; //this should be false here
}

Fragment_NRij_ao_ao_PardoLoopManager::Fragment_NRij_ao_ao_PardoLoopManager(
		int num_indices, const int (&index_id)[MAX_RANK],
		DataManager & data_manager, const SipTables & sip_tables,
		SIPMPIAttr & sip_mpi_attr, int num_where_clauses,
		Interpreter* interpreter, long& iteration) :
		FragmentPardoLoopManager(num_indices, index_id, data_manager,
				sip_tables), first_time_(true), iteration_(iteration), sip_mpi_attr_(
				sip_mpi_attr), num_where_clauses_(num_where_clauses), company_rank_(
				sip_mpi_attr.company_rank()), num_workers_(
				sip_mpi_attr_.num_workers()), interpreter_(interpreter) {

	//form_elst_dist();
	form_rcut_dist();
	form_swao_frag();
	form_swocca_frag();
	//form_swvirta_frag();
}

Fragment_NRij_ao_ao_PardoLoopManager::~Fragment_NRij_ao_ao_PardoLoopManager() {
}

/*!
 -------------------------------------------
 _NRij_vo_ao_
 -------------------------------------------
 */

/*
 for each new special fragment where clause pattern, this should be the only thing realy changed.
 see comment above for fragment_special_where_clause syntax
 */
bool Fragment_NRij_vo_ao_PardoLoopManager::where_clause(int index) {
	bool where_;
	int ifrag = 0;
	int jfrag = 1;
	int ao = 1;
	int occ = 2;
	int virt = 3;
	int elst = 4;
	int rcut = 5;
	int NE = 0;

	switch (index) {
	case 0:
	case 1:
		where_ = fragment_special_where_clause(rcut, jfrag, ifrag)
				&& fragment_special_where_clause(NE, jfrag, ifrag);
		break;
	case 2:
		where_ = fragment_special_where_clause(virt, index, ifrag);
		break;
	case 3:
		where_ = fragment_special_where_clause(occ, index, ifrag);
		break;
	case 4:
		where_ = fragment_special_where_clause(ao, index, jfrag);
		break;
	case 5:
		where_ = fragment_special_where_clause(occ, index, jfrag);
		break;
	default:
		where_ = false;
	}
	return where_;
}

bool Fragment_NRij_vo_ao_PardoLoopManager::do_update() {
	if (to_exit_)
		return false;
	bool more_iters;
	bool where_clauses_value;

	interpreter_->skip_where_clauses(num_where_clauses_);

	int loop_count = iteration_;
	int index_restart[MAX_RANK];

	if (first_time_) {
		first_time_ = false;
		more_iters = initialize_indices();

		for (int i = 0; i < num_indices_; ++i) {
			index_restart[i] = index_values_[i];
		}

		for (int index_i = index_restart[0]; index_i < upper_bound_[0];
				++index_i) {
			index_values_[0] = index_i;
			data_manager_.set_index_value(index_id_[0], index_i);

			for (int index_j = index_restart[1]; index_j < upper_bound_[1];
					++index_j) {
				index_values_[1] = index_j;
				data_manager_.set_index_value(index_id_[1], index_j);

				if (where_clause(1)) {
					for (int index_2 = index_restart[2];
							index_2 < upper_bound_[2]; ++index_2) {
						index_values_[2] = index_2;
						data_manager_.set_index_value(index_id_[2], index_2);

						if (where_clause(2)) {
							for (int index_3 = index_restart[3];
									index_3 < upper_bound_[3]; ++index_3) {
								index_values_[3] = index_3;
								data_manager_.set_index_value(index_id_[3],
										index_3);

								if (where_clause(3)) {
									for (int index_4 = index_restart[4];
											index_4 < upper_bound_[4];
											++index_4) {
										index_values_[4] = index_4;
										data_manager_.set_index_value(
												index_id_[4], index_4);

										if (where_clause(4)) {
											for (int index_5 = index_restart[5];
													index_5 < upper_bound_[5];
													++index_5) {
												index_values_[5] = index_5;
												data_manager_.set_index_value(
														index_id_[5], index_5);

												if (where_clause(5)) {
													iteration_++;
													if ((iteration_ - 1)
															% num_workers_
															== company_rank_) {
														return true;
													}
												} // where 5
											} // index_5
											for (int i = 5; i < num_indices_;
													++i) {
												index_restart[i] =
														lower_seg_[i];
											}
										} // where 4
									} // index_4
									for (int i = 4; i < num_indices_; ++i) {
										index_restart[i] = lower_seg_[i];
									}
								} // where 3
							} // index_3
							for (int i = 3; i < num_indices_; ++i) {
								index_restart[i] = lower_seg_[i];
							}
						} // where 2
					} // index_2
					for (int i = 2; i < num_indices_; ++i) {
						index_restart[i] = lower_seg_[i];
					}
				} // where 1
			} // index_j
			for (int i = 1; i < num_indices_; ++i) {
				index_restart[i] = lower_seg_[i];
			}
		} // index_i
	}

	for (int i = 0; i < num_indices_; ++i) {
		index_restart[i] = index_values_[i];
	}

	for (int index_i = index_restart[0]; index_i < upper_bound_[0]; ++index_i) {
		index_values_[0] = index_i;
		data_manager_.set_index_value(index_id_[0], index_i);

		for (int index_j = index_restart[1]; index_j < upper_bound_[1];
				++index_j) {
			index_values_[1] = index_j;
			data_manager_.set_index_value(index_id_[1], index_j);

			if (where_clause(1)) {
				for (int index_2 = index_restart[2]; index_2 < upper_bound_[2];
						++index_2) {
					index_values_[2] = index_2;
					data_manager_.set_index_value(index_id_[2], index_2);

					if (where_clause(2)) {
						for (int index_3 = index_restart[3];
								index_3 < upper_bound_[3]; ++index_3) {
							index_values_[3] = index_3;
							data_manager_.set_index_value(index_id_[3],
									index_3);

							if (where_clause(3)) {
								for (int index_4 = index_restart[4];
										index_4 < upper_bound_[4]; ++index_4) {
									index_values_[4] = index_4;
									data_manager_.set_index_value(index_id_[4],
											index_4);

									if (where_clause(4)) {
										for (int index_5 = index_restart[5];
												index_5 < upper_bound_[5];
												++index_5) {
											index_values_[5] = index_5;
											data_manager_.set_index_value(
													index_id_[5], index_5);

											if (where_clause(5)) {
												if (loop_count > iteration_) {
													iteration_++;
													if ((iteration_ - 1)
															% num_workers_
															== company_rank_) {
														return true;
													}
												}
												++loop_count;
											} // where 5
										} // index_5
										for (int i = 5; i < num_indices_; ++i) {
											index_restart[i] = lower_seg_[i];
										}
									} // where 4
								} // index_4
								for (int i = 4; i < num_indices_; ++i) {
									index_restart[i] = lower_seg_[i];
								}
							} // where 3
						} // index_3
						for (int i = 3; i < num_indices_; ++i) {
							index_restart[i] = lower_seg_[i];
						}
					} // where 2
				} // index_2
				for (int i = 2; i < num_indices_; ++i) {
					index_restart[i] = lower_seg_[i];
				}
			} // where 1
		} // index_j
		for (int i = 1; i < num_indices_; ++i) {
			index_restart[i] = lower_seg_[i];
		}
	} // index_i

	return false; //this should be false here
}

Fragment_NRij_vo_ao_PardoLoopManager::Fragment_NRij_vo_ao_PardoLoopManager(
		int num_indices, const int (&index_id)[MAX_RANK],
		DataManager & data_manager, const SipTables & sip_tables,
		SIPMPIAttr & sip_mpi_attr, int num_where_clauses,
		Interpreter* interpreter, long& iteration) :
		FragmentPardoLoopManager(num_indices, index_id, data_manager,
				sip_tables), first_time_(true), iteration_(iteration), sip_mpi_attr_(
				sip_mpi_attr), num_where_clauses_(num_where_clauses), company_rank_(
				sip_mpi_attr.company_rank()), num_workers_(
				sip_mpi_attr_.num_workers()), interpreter_(interpreter) {

	//form_elst_dist();
	form_rcut_dist();
	form_swao_frag();
	form_swocca_frag();
	form_swvirta_frag();
}

Fragment_NRij_vo_ao_PardoLoopManager::~Fragment_NRij_vo_ao_PardoLoopManager() {
}

/*!
 -------------------------------------------
 _NRij_aa_aa_
 -------------------------------------------
 */

/*
 for each new special fragment where clause pattern, this should be the only thing realy changed.
 see comment above for fragment_special_where_clause syntax
 */
bool Fragment_NRij_aa_aa_PardoLoopManager::where_clause(int index) {
	bool where_;
	int ifrag = 0;
	int jfrag = 1;
	int ao = 1;
	int occ = 2;
	int virt = 3;
	int elst = 4;
	int rcut = 5;
	int NE = 0;

	switch (index) {
	case 0:
	case 1:
		where_ = fragment_special_where_clause(rcut, jfrag, ifrag)
				&& fragment_special_where_clause(NE, jfrag, ifrag);
		break;
	case 2:
		where_ = fragment_special_where_clause(ao, index, ifrag);
		break;
	case 3:
		where_ = fragment_special_where_clause(ao, index, ifrag);
		break;
	case 4:
		where_ = fragment_special_where_clause(ao, index, jfrag);
		break;
	case 5:
		where_ = fragment_special_where_clause(ao, index, jfrag);
		break;
	default:
		where_ = false;
	}
	return where_;
}

bool Fragment_NRij_aa_aa_PardoLoopManager::do_update() {
	if (to_exit_)
		return false;
	bool more_iters;
	bool where_clauses_value;

	interpreter_->skip_where_clauses(num_where_clauses_);

	int loop_count = iteration_;
	int index_restart[MAX_RANK];

	if (first_time_) {
		first_time_ = false;
		more_iters = initialize_indices();

		for (int i = 0; i < num_indices_; ++i) {
			index_restart[i] = index_values_[i];
		}

		for (int index_i = index_restart[0]; index_i < upper_bound_[0];
				++index_i) {
			index_values_[0] = index_i;
			data_manager_.set_index_value(index_id_[0], index_i);

			for (int index_j = index_restart[1]; index_j < upper_bound_[1];
					++index_j) {
				index_values_[1] = index_j;
				data_manager_.set_index_value(index_id_[1], index_j);

				if (where_clause(1)) {
					for (int index_2 = index_restart[2];
							index_2 < upper_bound_[2]; ++index_2) {
						index_values_[2] = index_2;
						data_manager_.set_index_value(index_id_[2], index_2);

						if (where_clause(2)) {
							for (int index_3 = index_restart[3];
									index_3 < upper_bound_[3]; ++index_3) {
								index_values_[3] = index_3;
								data_manager_.set_index_value(index_id_[3],
										index_3);

								if (where_clause(3)) {
									for (int index_4 = index_restart[4];
											index_4 < upper_bound_[4];
											++index_4) {
										index_values_[4] = index_4;
										data_manager_.set_index_value(
												index_id_[4], index_4);

										if (where_clause(4)) {
											for (int index_5 = index_restart[5];
													index_5 < upper_bound_[5];
													++index_5) {
												index_values_[5] = index_5;
												data_manager_.set_index_value(
														index_id_[5], index_5);

												if (where_clause(5)) {
													iteration_++;
													if ((iteration_ - 1)
															% num_workers_
															== company_rank_) {
														return true;
													}
												} // where 5
											} // index_5
											for (int i = 5; i < num_indices_;
													++i) {
												index_restart[i] =
														lower_seg_[i];
											}
										} // where 4
									} // index_4
									for (int i = 4; i < num_indices_; ++i) {
										index_restart[i] = lower_seg_[i];
									}
								} // where 3
							} // index_3
							for (int i = 3; i < num_indices_; ++i) {
								index_restart[i] = lower_seg_[i];
							}
						} // where 2
					} // index_2
					for (int i = 2; i < num_indices_; ++i) {
						index_restart[i] = lower_seg_[i];
					}
				} // where 1
			} // index_j
			for (int i = 1; i < num_indices_; ++i) {
				index_restart[i] = lower_seg_[i];
			}
		} // index_i
	}

	for (int i = 0; i < num_indices_; ++i) {
		index_restart[i] = index_values_[i];
	}

	for (int index_i = index_restart[0]; index_i < upper_bound_[0]; ++index_i) {
		index_values_[0] = index_i;
		data_manager_.set_index_value(index_id_[0], index_i);

		for (int index_j = index_restart[1]; index_j < upper_bound_[1];
				++index_j) {
			index_values_[1] = index_j;
			data_manager_.set_index_value(index_id_[1], index_j);

			if (where_clause(1)) {
				for (int index_2 = index_restart[2]; index_2 < upper_bound_[2];
						++index_2) {
					index_values_[2] = index_2;
					data_manager_.set_index_value(index_id_[2], index_2);

					if (where_clause(2)) {
						for (int index_3 = index_restart[3];
								index_3 < upper_bound_[3]; ++index_3) {
							index_values_[3] = index_3;
							data_manager_.set_index_value(index_id_[3],
									index_3);

							if (where_clause(3)) {
								for (int index_4 = index_restart[4];
										index_4 < upper_bound_[4]; ++index_4) {
									index_values_[4] = index_4;
									data_manager_.set_index_value(index_id_[4],
											index_4);

									if (where_clause(4)) {
										for (int index_5 = index_restart[5];
												index_5 < upper_bound_[5];
												++index_5) {
											index_values_[5] = index_5;
											data_manager_.set_index_value(
													index_id_[5], index_5);

											if (where_clause(5)) {
												if (loop_count > iteration_) {
													iteration_++;
													if ((iteration_ - 1)
															% num_workers_
															== company_rank_) {
														return true;
													}
												}
												++loop_count;
											} // where 5
										} // index_5
										for (int i = 5; i < num_indices_; ++i) {
											index_restart[i] = lower_seg_[i];
										}
									} // where 4
								} // index_4
								for (int i = 4; i < num_indices_; ++i) {
									index_restart[i] = lower_seg_[i];
								}
							} // where 3
						} // index_3
						for (int i = 3; i < num_indices_; ++i) {
							index_restart[i] = lower_seg_[i];
						}
					} // where 2
				} // index_2
				for (int i = 2; i < num_indices_; ++i) {
					index_restart[i] = lower_seg_[i];
				}
			} // where 1
		} // index_j
		for (int i = 1; i < num_indices_; ++i) {
			index_restart[i] = lower_seg_[i];
		}
	} // index_i

	return false; //this should be false here
}

Fragment_NRij_aa_aa_PardoLoopManager::Fragment_NRij_aa_aa_PardoLoopManager(
		int num_indices, const int (&index_id)[MAX_RANK],
		DataManager & data_manager, const SipTables & sip_tables,
		SIPMPIAttr & sip_mpi_attr, int num_where_clauses,
		Interpreter* interpreter, long& iteration) :
		FragmentPardoLoopManager(num_indices, index_id, data_manager,
				sip_tables), first_time_(true), iteration_(iteration), sip_mpi_attr_(
				sip_mpi_attr), num_where_clauses_(num_where_clauses), company_rank_(
				sip_mpi_attr.company_rank()), num_workers_(
				sip_mpi_attr_.num_workers()), interpreter_(interpreter) {

	//form_elst_dist();
	form_rcut_dist();
	form_swao_frag();
	//form_swocca_frag();
	//form_swvirta_frag();
}

Fragment_NRij_aa_aa_PardoLoopManager::~Fragment_NRij_aa_aa_PardoLoopManager() {
}

/*!
 -------------------------------------------
 _NRij_o_ao_
 -------------------------------------------
 */

/*
 for each new special fragment where clause pattern, this should be the only thing realy changed.
 see comment above for fragment_special_where_clause syntax
 */
bool Fragment_NRij_o_ao_PardoLoopManager::where_clause(int index) {
	bool where_;
	int ifrag = 0;
	int jfrag = 1;
	int ao = 1;
	int occ = 2;
	int virt = 3;
	int elst = 4;
	int rcut = 5;
	int NE = 0;

	switch (index) {
	case 0:
	case 1:
		where_ = fragment_special_where_clause(rcut, jfrag, ifrag)
				&& fragment_special_where_clause(NE, jfrag, ifrag);
		break;
	case 2:
		where_ = fragment_special_where_clause(occ, index, ifrag);
		break;
	case 3:
		where_ = fragment_special_where_clause(ao, index, jfrag);
		break;
	case 4:
		where_ = fragment_special_where_clause(occ, index, jfrag);
		break;
	case 5:
		break;
	default:
		where_ = false;
	}
	return where_;
}

bool Fragment_NRij_o_ao_PardoLoopManager::do_update() {
	if (to_exit_)
		return false;
	bool more_iters;
	bool where_clauses_value;

	interpreter_->skip_where_clauses(num_where_clauses_);

	int loop_count = iteration_;
	int index_restart[MAX_RANK];

	if (first_time_) {
		first_time_ = false;
		more_iters = initialize_indices();

		for (int i = 0; i < num_indices_; ++i) {
			index_restart[i] = index_values_[i];
		}

		for (int index_i = index_restart[0]; index_i < upper_bound_[0];
				++index_i) {
			index_values_[0] = index_i;
			data_manager_.set_index_value(index_id_[0], index_i);

			for (int index_j = index_restart[1]; index_j < upper_bound_[1];
					++index_j) {
				index_values_[1] = index_j;
				data_manager_.set_index_value(index_id_[1], index_j);

				if (where_clause(1)) {
					for (int index_2 = index_restart[2];
							index_2 < upper_bound_[2]; ++index_2) {
						index_values_[2] = index_2;
						data_manager_.set_index_value(index_id_[2], index_2);

						if (where_clause(2)) {
							for (int index_3 = index_restart[3];
									index_3 < upper_bound_[3]; ++index_3) {
								index_values_[3] = index_3;
								data_manager_.set_index_value(index_id_[3],
										index_3);

								if (where_clause(3)) {
									for (int index_4 = index_restart[4];
											index_4 < upper_bound_[4];
											++index_4) {
										index_values_[4] = index_4;
										data_manager_.set_index_value(
												index_id_[4], index_4);

										if (where_clause(4)) {
											iteration_++;
											if ((iteration_ - 1) % num_workers_
													== company_rank_) {
												return true;
											}
										} // where 4
									} // index_4
									for (int i = 4; i < num_indices_; ++i) {
										index_restart[i] = lower_seg_[i];
									}
								} // where 3
							} // index_3
							for (int i = 3; i < num_indices_; ++i) {
								index_restart[i] = lower_seg_[i];
							}
						} // where 2
					} // index_2
					for (int i = 2; i < num_indices_; ++i) {
						index_restart[i] = lower_seg_[i];
					}
				} // where 1
			} // index_j
			for (int i = 1; i < num_indices_; ++i) {
				index_restart[i] = lower_seg_[i];
			}
		} // index_i
	}

	for (int i = 0; i < num_indices_; ++i) {
		index_restart[i] = index_values_[i];
	}

	for (int index_i = index_restart[0]; index_i < upper_bound_[0]; ++index_i) {
		index_values_[0] = index_i;
		data_manager_.set_index_value(index_id_[0], index_i);

		for (int index_j = index_restart[1]; index_j < upper_bound_[1];
				++index_j) {
			index_values_[1] = index_j;
			data_manager_.set_index_value(index_id_[1], index_j);

			if (where_clause(1)) {
				for (int index_2 = index_restart[2]; index_2 < upper_bound_[2];
						++index_2) {
					index_values_[2] = index_2;
					data_manager_.set_index_value(index_id_[2], index_2);

					if (where_clause(2)) {
						for (int index_3 = index_restart[3];
								index_3 < upper_bound_[3]; ++index_3) {
							index_values_[3] = index_3;
							data_manager_.set_index_value(index_id_[3],
									index_3);

							if (where_clause(3)) {
								for (int index_4 = index_restart[4];
										index_4 < upper_bound_[4]; ++index_4) {
									index_values_[4] = index_4;
									data_manager_.set_index_value(index_id_[4],
											index_4);

									if (where_clause(4)) {
										if (loop_count > iteration_) {
											iteration_++;
											if ((iteration_ - 1) % num_workers_
													== company_rank_) {
												return true;
											}
										}
										++loop_count;
									} // where 4
								} // index_4
								for (int i = 4; i < num_indices_; ++i) {
									index_restart[i] = lower_seg_[i];
								}
							} // where 3
						} // index_3
						for (int i = 3; i < num_indices_; ++i) {
							index_restart[i] = lower_seg_[i];
						}
					} // where 2
				} // index_2
				for (int i = 2; i < num_indices_; ++i) {
					index_restart[i] = lower_seg_[i];
				}
			} // where 1
		} // index_j
		for (int i = 1; i < num_indices_; ++i) {
			index_restart[i] = lower_seg_[i];
		}
	} // index_i

	return false; //this should be false here
}

Fragment_NRij_o_ao_PardoLoopManager::Fragment_NRij_o_ao_PardoLoopManager(
		int num_indices, const int (&index_id)[MAX_RANK],
		DataManager & data_manager, const SipTables & sip_tables,
		SIPMPIAttr & sip_mpi_attr, int num_where_clauses,
		Interpreter* interpreter, long& iteration) :
		FragmentPardoLoopManager(num_indices, index_id, data_manager,
				sip_tables), first_time_(true), iteration_(iteration), sip_mpi_attr_(
				sip_mpi_attr), num_where_clauses_(num_where_clauses), company_rank_(
				sip_mpi_attr.company_rank()), num_workers_(
				sip_mpi_attr_.num_workers()), interpreter_(interpreter) {

	//form_elst_dist();
	form_rcut_dist();
	form_swao_frag();
	form_swocca_frag();
	//form_swvirta_frag();
}

Fragment_NRij_o_ao_PardoLoopManager::~Fragment_NRij_o_ao_PardoLoopManager() {
}

/*!
 -------------------------------------------
 _ij_aa_vo_
 -------------------------------------------
 */

/*
 for each new special fragment where clause pattern, this should be the only thing realy changed.
 see comment above for fragment_special_where_clause syntax
 */
bool Fragment_ij_aa_vo_PardoLoopManager::where_clause(int index) {
	bool where_;
	int ifrag = 0;
	int jfrag = 1;
	int ao = 1;
	int occ = 2;
	int virt = 3;
	int elst = 4;
	int rcut = 5;

	switch (index) {
	case 0:
	case 1:
		where_ = fragment_special_where_clause(elst, jfrag, ifrag);
		break;
	case 2:
		where_ = fragment_special_where_clause(ao, index, ifrag);
		break;
	case 3:
		where_ = fragment_special_where_clause(ao, index, ifrag);
		break;
	case 4:
		where_ = fragment_special_where_clause(virt, index, jfrag);
		break;
	case 5:
		where_ = fragment_special_where_clause(occ, index, jfrag);
		break;
	default:
		where_ = false;
	}
	return where_;
}

bool Fragment_ij_aa_vo_PardoLoopManager::do_update() {
	if (to_exit_)
		return false;
	bool more_iters;
	bool where_clauses_value;

	interpreter_->skip_where_clauses(num_where_clauses_);

	if (first_time_) {
		first_time_ = false;
		more_iters = initialize_indices();

		where_clauses_value = true;
		for (int i = 1; i < num_indices_; ++i) {
			where_clauses_value = where_clauses_value && where_clause(i);
		}
		if (where_clauses_value) {
			iteration_++;
			if ((iteration_ - 1) % num_workers_ == company_rank_) {
				return true;
			}
		}
	}

	int index_restart[MAX_RANK];

	for (int i = 0; i < num_indices_; ++i) {
		index_restart[i] = index_values_[i];
	}

	int loop_count = iteration_;
	for (int index_i = index_restart[0]; index_i < upper_bound_[0]; ++index_i) {
		index_values_[0] = index_i;
		data_manager_.set_index_value(index_id_[0], index_i);

		for (int index_j = index_restart[1]; index_j < upper_bound_[1];
				++index_j) {
			index_values_[1] = index_j;
			data_manager_.set_index_value(index_id_[1], index_j);

			if (where_clause(1)) {
				for (int index_2 = index_restart[2]; index_2 < upper_bound_[2];
						++index_2) {
					index_values_[2] = index_2;
					data_manager_.set_index_value(index_id_[2], index_2);

					if (where_clause(2)) {
						for (int index_3 = index_restart[3];
								index_3 < upper_bound_[3]; ++index_3) {
							index_values_[3] = index_3;
							data_manager_.set_index_value(index_id_[3],
									index_3);

							if (where_clause(3)) {
								for (int index_4 = index_restart[4];
										index_4 < upper_bound_[4]; ++index_4) {
									index_values_[4] = index_4;
									data_manager_.set_index_value(index_id_[4],
											index_4);

									if (where_clause(4)) {
										for (int index_5 = index_restart[5];
												index_5 < upper_bound_[5];
												++index_5) {
											index_values_[5] = index_5;
											data_manager_.set_index_value(
													index_id_[5], index_5);

											if (where_clause(5)) {
												if (loop_count > iteration_) {
													iteration_++;
													if ((iteration_ - 1)
															% num_workers_
															== company_rank_) {
														return true;
													}
												}
												++loop_count;
											} // where 5
										} // index_5
										for (int i = 5; i < num_indices_; ++i) {
											index_restart[i] = lower_seg_[i];
										}
									} // where 4
								} // index_4
								for (int i = 4; i < num_indices_; ++i) {
									index_restart[i] = lower_seg_[i];
								}
							} // where 3
						} // index_3
						for (int i = 3; i < num_indices_; ++i) {
							index_restart[i] = lower_seg_[i];
						}
					} // where 2
				} // index_2
				for (int i = 2; i < num_indices_; ++i) {
					index_restart[i] = lower_seg_[i];
				}
			} // where 1
		} // index_j
		for (int i = 1; i < num_indices_; ++i) {
			index_restart[i] = lower_seg_[i];
		}
	} // index_i

	return false; //this should be false here
}

Fragment_ij_aa_vo_PardoLoopManager::Fragment_ij_aa_vo_PardoLoopManager(
		int num_indices, const int (&index_id)[MAX_RANK],
		DataManager & data_manager, const SipTables & sip_tables,
		SIPMPIAttr & sip_mpi_attr, int num_where_clauses,
		Interpreter* interpreter, long& iteration) :
		FragmentPardoLoopManager(num_indices, index_id, data_manager,
				sip_tables), first_time_(true), iteration_(iteration), sip_mpi_attr_(
				sip_mpi_attr), num_where_clauses_(num_where_clauses), company_rank_(
				sip_mpi_attr.company_rank()), num_workers_(
				sip_mpi_attr_.num_workers()), interpreter_(interpreter) {

	form_elst_dist();
	//form_rcut_dist();
	form_swao_frag();
	form_swocca_frag();
	form_swvirta_frag();
}

Fragment_ij_aa_vo_PardoLoopManager::~Fragment_ij_aa_vo_PardoLoopManager() {
}

/*!
 -------------------------------------------
 _i_pppp__
 -------------------------------------------
 */

/*
 for each new special fragment where clause pattern, this should be the only thing realy changed.
 see comment above for fragment_special_where_clause syntax
 */
bool Fragment_i_pppp__PardoLoopManager::where_clause(int index) {
	bool where_;
	int ifrag = 0;
	int jfrag = 1;
	int ao = 1;
	int occ = 2;
	int virt = 3;
	int elst = 4;
	int rcut = 5;
	int mo = 6;

	switch (index) {
	case 0:
		where_ = true;
		break;
	case 1:
		where_ = fragment_special_where_clause(mo, index, ifrag);
		break;
	case 2:
		where_ = fragment_special_where_clause(mo, index, ifrag);
		break;
	case 3:
		where_ = fragment_special_where_clause(mo, index, ifrag);
		break;
	case 4:
		where_ = fragment_special_where_clause(mo, index, ifrag);
		break;
	case 5:
		break;
	default:
		where_ = false;
	}
	return where_;
}

bool Fragment_i_pppp__PardoLoopManager::do_update() {
	if (to_exit_)
		return false;
	bool more_iters;
	bool where_clauses_value;

	interpreter_->skip_where_clauses(num_where_clauses_);

	if (first_time_) {
		first_time_ = false;
		more_iters = initialize_indices();

		where_clauses_value = true;
		for (int i = 1; i < num_indices_; ++i) {
			where_clauses_value = where_clauses_value && where_clause(i);
		}
		if (where_clauses_value) {
			iteration_++;
			if ((iteration_ - 1) % num_workers_ == company_rank_) {
				return true;
			}
		}
	}

	int index_restart[MAX_RANK];

	for (int i = 0; i < num_indices_; ++i) {
		index_restart[i] = index_values_[i];
	}

	int loop_count = iteration_;
	for (int index_i = index_restart[0]; index_i < upper_bound_[0]; ++index_i) {
		index_values_[0] = index_i;
		data_manager_.set_index_value(index_id_[0], index_i);

		for (int index_1 = index_restart[1]; index_1 < upper_bound_[1];
				++index_1) {
			index_values_[1] = index_1;
			data_manager_.set_index_value(index_id_[1], index_1);

			if (where_clause(1)) {
				for (int index_2 = index_restart[2]; index_2 < upper_bound_[2];
						++index_2) {
					index_values_[2] = index_2;
					data_manager_.set_index_value(index_id_[2], index_2);

					if (where_clause(2)) {
						for (int index_3 = index_restart[3];
								index_3 < upper_bound_[3]; ++index_3) {
							index_values_[3] = index_3;
							data_manager_.set_index_value(index_id_[3],
									index_3);

							if (where_clause(3)) {
								for (int index_4 = index_restart[4];
										index_4 < upper_bound_[4]; ++index_4) {
									index_values_[4] = index_4;
									data_manager_.set_index_value(index_id_[4],
											index_4);

									if (where_clause(4)) {
										if (loop_count > iteration_) {
											iteration_++;
											if ((iteration_ - 1) % num_workers_
													== company_rank_) {
												return true;
											}
										}
										++loop_count;
									} // where 4
								} // index_4
								for (int i = 4; i < num_indices_; ++i) {
									index_restart[i] = lower_seg_[i];
								}
							} // where 3
						} // index_3
						for (int i = 3; i < num_indices_; ++i) {
							index_restart[i] = lower_seg_[i];
						}
					} // where 2
				} // index_2
				for (int i = 2; i < num_indices_; ++i) {
					index_restart[i] = lower_seg_[i];
				}
			} // where 1
		} // index_1
		for (int i = 1; i < num_indices_; ++i) {
			index_restart[i] = lower_seg_[i];
		}
	} // index_i

	return false; //this should be false here
}

Fragment_i_pppp__PardoLoopManager::Fragment_i_pppp__PardoLoopManager(
		int num_indices, const int (&index_id)[MAX_RANK],
		DataManager & data_manager, const SipTables & sip_tables,
		SIPMPIAttr & sip_mpi_attr, int num_where_clauses,
		Interpreter* interpreter, long& iteration) :
		FragmentPardoLoopManager(num_indices, index_id, data_manager,
				sip_tables), first_time_(true), iteration_(iteration), sip_mpi_attr_(
				sip_mpi_attr), num_where_clauses_(num_where_clauses), company_rank_(
				sip_mpi_attr.company_rank()), num_workers_(
				sip_mpi_attr_.num_workers()), interpreter_(interpreter) {

	form_swmoa_frag();
}

Fragment_i_pppp__PardoLoopManager::~Fragment_i_pppp__PardoLoopManager() {
}

/*!
 -------------------------------------------
 _i_pp__
 -------------------------------------------

 PARDO ifrag, mu,....#GETLINE: Fragment_i_pp__
 where (int)SwAO_frag[(index)mu] == ifrag
 .
 .
 .
 */

/*
 for each new special fragment where clause pattern, this should be the only thing realy changed.
 see comment above for fragment_special_where_clause syntax
 */
bool Fragment_i_pp__PardoLoopManager::where_clause(int index) {
	bool where_;
	int ifrag = 0;
	int jfrag = 1;
	int ao = 1;
	int occ = 2;
	int virt = 3;
	int elst = 4;
	int rcut = 5;
	int mo = 6;

	switch (index) {
	case 0:
		where_ = true;
		break;
	case 1:
		where_ = fragment_special_where_clause(mo, index, ifrag);
		break;
	case 2:
		where_ = fragment_special_where_clause(mo, index, ifrag);
		break;
	case 3:
	case 4:
	case 5:
		break;
	default:
		where_ = false;
	}
	return where_;
}

bool Fragment_i_pp__PardoLoopManager::do_update() {
	if (to_exit_)
		return false;
	bool more_iters;
	bool where_clauses_value;

	interpreter_->skip_where_clauses(num_where_clauses_);

	if (first_time_) {
		first_time_ = false;
		more_iters = initialize_indices();

		where_clauses_value = true;
		for (int i = 1; i < num_indices_; ++i) {
			where_clauses_value = where_clauses_value && where_clause(i);
		}
		if (where_clauses_value) {
			iteration_++;
			if ((iteration_ - 1) % num_workers_ == company_rank_) {
				return true;
			}
		}
	}

	int index_restart[MAX_RANK];

	for (int i = 0; i < num_indices_; ++i) {
		index_restart[i] = index_values_[i];
	}

	int loop_count = iteration_;
	for (int index_i = index_restart[0]; index_i < upper_bound_[0]; ++index_i) {
		index_values_[0] = index_i;
		data_manager_.set_index_value(index_id_[0], index_i);

		for (int index_1 = index_restart[1]; index_1 < upper_bound_[1];
				++index_1) {
			index_values_[1] = index_1;
			data_manager_.set_index_value(index_id_[1], index_1);

			if (where_clause(1)) {
				for (int index_2 = index_restart[2]; index_2 < upper_bound_[2];
						++index_2) {
					index_values_[2] = index_2;
					data_manager_.set_index_value(index_id_[2], index_2);

					if (where_clause(2)) {
						if (loop_count > iteration_) {
							iteration_++;
							if ((iteration_ - 1) % num_workers_
									== company_rank_) {
								return true;
							}
						}
						++loop_count;
					} // where 2
				} // index_2
				for (int i = 2; i < num_indices_; ++i) {
					index_restart[i] = lower_seg_[i];
				}
			} // where 1
		} // index_1
		for (int i = 1; i < num_indices_; ++i) {
			index_restart[i] = lower_seg_[i];
		}
	} // index_i

	return false; //this should be false here
}

Fragment_i_pp__PardoLoopManager::Fragment_i_pp__PardoLoopManager(
		int num_indices, const int (&index_id)[MAX_RANK],
		DataManager & data_manager, const SipTables & sip_tables,
		SIPMPIAttr & sip_mpi_attr, int num_where_clauses,
		Interpreter* interpreter, long& iteration) :
		FragmentPardoLoopManager(num_indices, index_id, data_manager,
				sip_tables), first_time_(true), iteration_(iteration), sip_mpi_attr_(
				sip_mpi_attr), num_where_clauses_(num_where_clauses), company_rank_(
				sip_mpi_attr.company_rank()), num_workers_(
				sip_mpi_attr_.num_workers()), interpreter_(interpreter) {

	form_swmoa_frag();
}

Fragment_i_pp__PardoLoopManager::~Fragment_i_pp__PardoLoopManager() {
}

/*!
 -------------------------------------------
 _ij_ap_pp_
 -------------------------------------------
 */

/*
 for each new special fragment where clause pattern, this should be the only thing realy changed.
 see comment above for fragment_special_where_clause syntax
 */
bool Fragment_ij_ap_pp_PardoLoopManager::where_clause(int index) {
	bool where_;
	int ifrag = 0;
	int jfrag = 1;
	int ao = 1;
	int occ = 2;
	int virt = 3;
	int elst = 4;
	int rcut = 5;
	int mo = 6;

	switch (index) {
	case 0:
	case 1:
		where_ = fragment_special_where_clause(elst, jfrag, ifrag);
		break;
	case 2:
		where_ = fragment_special_where_clause(ao, index, ifrag);
		break;
	case 3:
		where_ = fragment_special_where_clause(mo, index, ifrag);
		break;
	case 4:
		where_ = fragment_special_where_clause(mo, index, jfrag);
		break;
	case 5:
		where_ = fragment_special_where_clause(mo, index, jfrag);
		break;
	default:
		where_ = false;
	}
	return where_;
}

bool Fragment_ij_ap_pp_PardoLoopManager::do_update() {
	if (to_exit_)
		return false;
	bool more_iters;
	bool where_clauses_value;

	interpreter_->skip_where_clauses(num_where_clauses_);

	if (first_time_) {
		first_time_ = false;
		more_iters = initialize_indices();

		where_clauses_value = true;
		for (int i = 1; i < num_indices_; ++i) {
			where_clauses_value = where_clauses_value && where_clause(i);
		}
		if (where_clauses_value) {
			iteration_++;
			if ((iteration_ - 1) % num_workers_ == company_rank_) {
				return true;
			}
		}
	}

	int index_restart[MAX_RANK];

	for (int i = 0; i < num_indices_; ++i) {
		index_restart[i] = index_values_[i];
	}

	int loop_count = iteration_;
	for (int index_i = index_restart[0]; index_i < upper_bound_[0]; ++index_i) {
		index_values_[0] = index_i;
		data_manager_.set_index_value(index_id_[0], index_i);

		for (int index_j = index_restart[1]; index_j < upper_bound_[1];
				++index_j) {
			index_values_[1] = index_j;
			data_manager_.set_index_value(index_id_[1], index_j);

			if (where_clause(1)) {
				for (int index_2 = index_restart[2]; index_2 < upper_bound_[2];
						++index_2) {
					index_values_[2] = index_2;
					data_manager_.set_index_value(index_id_[2], index_2);

					if (where_clause(2)) {
						for (int index_3 = index_restart[3];
								index_3 < upper_bound_[3]; ++index_3) {
							index_values_[3] = index_3;
							data_manager_.set_index_value(index_id_[3],
									index_3);

							if (where_clause(3)) {
								for (int index_4 = index_restart[4];
										index_4 < upper_bound_[4]; ++index_4) {
									index_values_[4] = index_4;
									data_manager_.set_index_value(index_id_[4],
											index_4);

									if (where_clause(4)) {
										for (int index_5 = index_restart[5];
												index_5 < upper_bound_[5];
												++index_5) {
											index_values_[5] = index_5;
											data_manager_.set_index_value(
													index_id_[5], index_5);

											if (where_clause(5)) {
												if (loop_count > iteration_) {
													iteration_++;
													if ((iteration_ - 1)
															% num_workers_
															== company_rank_) {
														return true;
													}
												}
												++loop_count;
											} // where 5
										} // index_5
										for (int i = 5; i < num_indices_; ++i) {
											index_restart[i] = lower_seg_[i];
										}
									} // where 4
								} // index_4
								for (int i = 4; i < num_indices_; ++i) {
									index_restart[i] = lower_seg_[i];
								}
							} // where 3
						} // index_3
						for (int i = 3; i < num_indices_; ++i) {
							index_restart[i] = lower_seg_[i];
						}
					} // where 2
				} // index_2
				for (int i = 2; i < num_indices_; ++i) {
					index_restart[i] = lower_seg_[i];
				}
			} // where 1
		} // index_j
		for (int i = 1; i < num_indices_; ++i) {
			index_restart[i] = lower_seg_[i];
		}
	} // index_i

	return false; //this should be false here
}

Fragment_ij_ap_pp_PardoLoopManager::Fragment_ij_ap_pp_PardoLoopManager(
		int num_indices, const int (&index_id)[MAX_RANK],
		DataManager & data_manager, const SipTables & sip_tables,
		SIPMPIAttr & sip_mpi_attr, int num_where_clauses,
		Interpreter* interpreter, long& iteration) :
		FragmentPardoLoopManager(num_indices, index_id, data_manager,
				sip_tables), first_time_(true), iteration_(iteration), sip_mpi_attr_(
				sip_mpi_attr), num_where_clauses_(num_where_clauses), company_rank_(
				sip_mpi_attr.company_rank()), num_workers_(
				sip_mpi_attr_.num_workers()), interpreter_(interpreter) {

	form_elst_dist();
	form_swao_frag();
	form_swmoa_frag();
}

Fragment_ij_ap_pp_PardoLoopManager::~Fragment_ij_ap_pp_PardoLoopManager() {
}

/*!
 -------------------------------------------
 _ij_pp_pp_
 -------------------------------------------
 */

/*
 for each new special fragment where clause pattern, this should be the only thing realy changed.
 see comment above for fragment_special_where_clause syntax
 */
bool Fragment_ij_pp_pp_PardoLoopManager::where_clause(int index) {
	bool where_;
	int ifrag = 0;
	int jfrag = 1;
	int ao = 1;
	int occ = 2;
	int virt = 3;
	int elst = 4;
	int rcut = 5;
	int mo = 6;

	switch (index) {
	case 0:
	case 1:
		where_ = fragment_special_where_clause(elst, jfrag, ifrag);
		break;
	case 2:
		where_ = fragment_special_where_clause(mo, index, ifrag);
		break;
	case 3:
		where_ = fragment_special_where_clause(mo, index, ifrag);
		break;
	case 4:
		where_ = fragment_special_where_clause(mo, index, jfrag);
		break;
	case 5:
		where_ = fragment_special_where_clause(mo, index, jfrag);
		break;
	default:
		where_ = false;
	}
	return where_;
}

bool Fragment_ij_pp_pp_PardoLoopManager::do_update() {
	if (to_exit_)
		return false;
	bool more_iters;
	bool where_clauses_value;

	interpreter_->skip_where_clauses(num_where_clauses_);

	if (first_time_) {
		first_time_ = false;
		more_iters = initialize_indices();

		where_clauses_value = true;
		for (int i = 1; i < num_indices_; ++i) {
			where_clauses_value = where_clauses_value && where_clause(i);
		}
		if (where_clauses_value) {
			iteration_++;
			if ((iteration_ - 1) % num_workers_ == company_rank_) {
				return true;
			}
		}
	}

	int index_restart[MAX_RANK];

	for (int i = 0; i < num_indices_; ++i) {
		index_restart[i] = index_values_[i];
	}

	int loop_count = iteration_;
	for (int index_i = index_restart[0]; index_i < upper_bound_[0]; ++index_i) {
		index_values_[0] = index_i;
		data_manager_.set_index_value(index_id_[0], index_i);

		for (int index_j = index_restart[1]; index_j < upper_bound_[1];
				++index_j) {
			index_values_[1] = index_j;
			data_manager_.set_index_value(index_id_[1], index_j);

			if (where_clause(1)) {
				for (int index_2 = index_restart[2]; index_2 < upper_bound_[2];
						++index_2) {
					index_values_[2] = index_2;
					data_manager_.set_index_value(index_id_[2], index_2);

					if (where_clause(2)) {
						for (int index_3 = index_restart[3];
								index_3 < upper_bound_[3]; ++index_3) {
							index_values_[3] = index_3;
							data_manager_.set_index_value(index_id_[3],
									index_3);

							if (where_clause(3)) {
								for (int index_4 = index_restart[4];
										index_4 < upper_bound_[4]; ++index_4) {
									index_values_[4] = index_4;
									data_manager_.set_index_value(index_id_[4],
											index_4);

									if (where_clause(4)) {
										for (int index_5 = index_restart[5];
												index_5 < upper_bound_[5];
												++index_5) {
											index_values_[5] = index_5;
											data_manager_.set_index_value(
													index_id_[5], index_5);

											if (where_clause(5)) {
												if (loop_count > iteration_) {
													iteration_++;
													if ((iteration_ - 1)
															% num_workers_
															== company_rank_) {
														return true;
													}
												}
												++loop_count;
											} // where 5
										} // index_5
										for (int i = 5; i < num_indices_; ++i) {
											index_restart[i] = lower_seg_[i];
										}
									} // where 4
								} // index_4
								for (int i = 4; i < num_indices_; ++i) {
									index_restart[i] = lower_seg_[i];
								}
							} // where 3
						} // index_3
						for (int i = 3; i < num_indices_; ++i) {
							index_restart[i] = lower_seg_[i];
						}
					} // where 2
				} // index_2
				for (int i = 2; i < num_indices_; ++i) {
					index_restart[i] = lower_seg_[i];
				}
			} // where 1
		} // index_j
		for (int i = 1; i < num_indices_; ++i) {
			index_restart[i] = lower_seg_[i];
		}
	} // index_i

	return false; //this should be false here
}

Fragment_ij_pp_pp_PardoLoopManager::Fragment_ij_pp_pp_PardoLoopManager(
		int num_indices, const int (&index_id)[MAX_RANK],
		DataManager & data_manager, const SipTables & sip_tables,
		SIPMPIAttr & sip_mpi_attr, int num_where_clauses,
		Interpreter* interpreter, long& iteration) :
		FragmentPardoLoopManager(num_indices, index_id, data_manager,
				sip_tables), first_time_(true), iteration_(iteration), sip_mpi_attr_(
				sip_mpi_attr), num_where_clauses_(num_where_clauses), company_rank_(
				sip_mpi_attr.company_rank()), num_workers_(
				sip_mpi_attr_.num_workers()), interpreter_(interpreter) {

	form_elst_dist();
	form_swao_frag();
	form_swmoa_frag();
}

Fragment_ij_pp_pp_PardoLoopManager::~Fragment_ij_pp_pp_PardoLoopManager() {
}

/*!
 -------------------------------------------
 _Nij_pp_pp_
 -------------------------------------------
 */

/*
 for each new special fragment where clause pattern, this should be the only thing realy changed.
 see comment above for fragment_special_where_clause syntax
 */
bool Fragment_Nij_pp_pp_PardoLoopManager::where_clause(int index) {
	bool where_;
	int ifrag = 0;
	int jfrag = 1;
	int ao = 1;
	int occ = 2;
	int virt = 3;
	int elst = 4;
	int rcut = 5;
	int NE = 0;
	int mo = 6;

	switch (index) {
	case 0:
	case 1:
		where_ = fragment_special_where_clause(elst, jfrag, ifrag)
				&& fragment_special_where_clause(NE, jfrag, ifrag);
		break;
	case 2:
		where_ = fragment_special_where_clause(mo, index, ifrag);
		break;
	case 3:
		where_ = fragment_special_where_clause(mo, index, ifrag);
		break;
	case 4:
		where_ = fragment_special_where_clause(mo, index, jfrag);
		break;
	case 5:
		where_ = fragment_special_where_clause(mo, index, jfrag);
		break;
	default:
		where_ = false;
	}
	return where_;
}

bool Fragment_Nij_pp_pp_PardoLoopManager::do_update() {
	if (to_exit_)
		return false;
	bool more_iters;
	bool where_clauses_value;

	interpreter_->skip_where_clauses(num_where_clauses_);

	int loop_count = iteration_;
	int index_restart[MAX_RANK];

	if (first_time_) {
		first_time_ = false;
		more_iters = initialize_indices();

		for (int i = 0; i < num_indices_; ++i) {
			index_restart[i] = index_values_[i];
		}

		for (int index_i = index_restart[0]; index_i < upper_bound_[0];
				++index_i) {
			index_values_[0] = index_i;
			data_manager_.set_index_value(index_id_[0], index_i);

			for (int index_j = index_restart[1]; index_j < upper_bound_[1];
					++index_j) {
				index_values_[1] = index_j;
				data_manager_.set_index_value(index_id_[1], index_j);

				if (where_clause(1)) {
					for (int index_2 = index_restart[2];
							index_2 < upper_bound_[2]; ++index_2) {
						index_values_[2] = index_2;
						data_manager_.set_index_value(index_id_[2], index_2);

						if (where_clause(2)) {
							for (int index_3 = index_restart[3];
									index_3 < upper_bound_[3]; ++index_3) {
								index_values_[3] = index_3;
								data_manager_.set_index_value(index_id_[3],
										index_3);

								if (where_clause(3)) {
									for (int index_4 = index_restart[4];
											index_4 < upper_bound_[4];
											++index_4) {
										index_values_[4] = index_4;
										data_manager_.set_index_value(
												index_id_[4], index_4);

										if (where_clause(4)) {
											for (int index_5 = index_restart[5];
													index_5 < upper_bound_[5];
													++index_5) {
												index_values_[5] = index_5;
												data_manager_.set_index_value(
														index_id_[5], index_5);

												if (where_clause(5)) {
													iteration_++;
													if ((iteration_ - 1)
															% num_workers_
															== company_rank_) {
														return true;
													}
												} // where 5
											} // index_5
											for (int i = 5; i < num_indices_;
													++i) {
												index_restart[i] =
														lower_seg_[i];
											}
										} // where 4
									} // index_4
									for (int i = 4; i < num_indices_; ++i) {
										index_restart[i] = lower_seg_[i];
									}
								} // where 3
							} // index_3
							for (int i = 3; i < num_indices_; ++i) {
								index_restart[i] = lower_seg_[i];
							}
						} // where 2
					} // index_2
					for (int i = 2; i < num_indices_; ++i) {
						index_restart[i] = lower_seg_[i];
					}
				} // where 1
			} // index_j
			for (int i = 1; i < num_indices_; ++i) {
				index_restart[i] = lower_seg_[i];
			}
		} // index_i
	}

	for (int i = 0; i < num_indices_; ++i) {
		index_restart[i] = index_values_[i];
	}

	for (int index_i = index_restart[0]; index_i < upper_bound_[0]; ++index_i) {
		index_values_[0] = index_i;
		data_manager_.set_index_value(index_id_[0], index_i);

		for (int index_j = index_restart[1]; index_j < upper_bound_[1];
				++index_j) {
			index_values_[1] = index_j;
			data_manager_.set_index_value(index_id_[1], index_j);

			if (where_clause(1)) {
				for (int index_2 = index_restart[2]; index_2 < upper_bound_[2];
						++index_2) {
					index_values_[2] = index_2;
					data_manager_.set_index_value(index_id_[2], index_2);

					if (where_clause(2)) {
						for (int index_3 = index_restart[3];
								index_3 < upper_bound_[3]; ++index_3) {
							index_values_[3] = index_3;
							data_manager_.set_index_value(index_id_[3],
									index_3);

							if (where_clause(3)) {
								for (int index_4 = index_restart[4];
										index_4 < upper_bound_[4]; ++index_4) {
									index_values_[4] = index_4;
									data_manager_.set_index_value(index_id_[4],
											index_4);

									if (where_clause(4)) {
										for (int index_5 = index_restart[5];
												index_5 < upper_bound_[5];
												++index_5) {
											index_values_[5] = index_5;
											data_manager_.set_index_value(
													index_id_[5], index_5);

											if (where_clause(5)) {
												if (loop_count > iteration_) {
													iteration_++;
													if ((iteration_ - 1)
															% num_workers_
															== company_rank_) {
														return true;
													}
												}
												++loop_count;
											} // where 5
										} // index_5
										for (int i = 5; i < num_indices_; ++i) {
											index_restart[i] = lower_seg_[i];
										}
									} // where 4
								} // index_4
								for (int i = 4; i < num_indices_; ++i) {
									index_restart[i] = lower_seg_[i];
								}
							} // where 3
						} // index_3
						for (int i = 3; i < num_indices_; ++i) {
							index_restart[i] = lower_seg_[i];
						}
					} // where 2
				} // index_2
				for (int i = 2; i < num_indices_; ++i) {
					index_restart[i] = lower_seg_[i];
				}
			} // where 1
		} // index_j
		for (int i = 1; i < num_indices_; ++i) {
			index_restart[i] = lower_seg_[i];
		}
	} // index_i

	return false; //this should be false here
}

Fragment_Nij_pp_pp_PardoLoopManager::Fragment_Nij_pp_pp_PardoLoopManager(
		int num_indices, const int (&index_id)[MAX_RANK],
		DataManager & data_manager, const SipTables & sip_tables,
		SIPMPIAttr & sip_mpi_attr, int num_where_clauses,
		Interpreter* interpreter, long& iteration) :
		FragmentPardoLoopManager(num_indices, index_id, data_manager,
				sip_tables), first_time_(true), iteration_(iteration), sip_mpi_attr_(
				sip_mpi_attr), num_where_clauses_(num_where_clauses), company_rank_(
				sip_mpi_attr.company_rank()), num_workers_(
				sip_mpi_attr_.num_workers()), interpreter_(interpreter) {

	form_elst_dist();
	form_swmoa_frag();
}

Fragment_Nij_pp_pp_PardoLoopManager::~Fragment_Nij_pp_pp_PardoLoopManager() {
}

/*!
 -------------------------------------------
 _Rij_pp_pp_
 -------------------------------------------
 */

/*
 for each new special fragment where clause pattern, this should be the only thing realy changed.
 see comment above for fragment_special_where_clause syntax
 */
bool Fragment_Rij_pp_pp_PardoLoopManager::where_clause(int index) {
	bool where_;
	int ifrag = 0;
	int jfrag = 1;
	int ao = 1;
	int occ = 2;
	int virt = 3;
	int elst = 4;
	int rcut = 5;
	int NE = 0;
	int mo = 6;

	switch (index) {
	case 0:
	case 1:
		where_ = fragment_special_where_clause(rcut, jfrag, ifrag);
		break;
	case 2:
		where_ = fragment_special_where_clause(mo, index, ifrag);
		break;
	case 3:
		where_ = fragment_special_where_clause(mo, index, ifrag);
		break;
	case 4:
		where_ = fragment_special_where_clause(mo, index, jfrag);
		break;
	case 5:
		where_ = fragment_special_where_clause(mo, index, jfrag);
		break;
	default:
		where_ = false;
	}
	return where_;
}

bool Fragment_Rij_pp_pp_PardoLoopManager::do_update() {
	if (to_exit_)
		return false;
	bool more_iters;
	bool where_clauses_value;

	interpreter_->skip_where_clauses(num_where_clauses_);

	int loop_count = iteration_;
	int index_restart[MAX_RANK];

	if (first_time_) {
		first_time_ = false;
		more_iters = initialize_indices();

		for (int i = 0; i < num_indices_; ++i) {
			index_restart[i] = index_values_[i];
		}

		for (int index_i = index_restart[0]; index_i < upper_bound_[0];
				++index_i) {
			index_values_[0] = index_i;
			data_manager_.set_index_value(index_id_[0], index_i);

			for (int index_j = index_restart[1]; index_j < upper_bound_[1];
					++index_j) {
				index_values_[1] = index_j;
				data_manager_.set_index_value(index_id_[1], index_j);

				if (where_clause(1)) {
					for (int index_2 = index_restart[2];
							index_2 < upper_bound_[2]; ++index_2) {
						index_values_[2] = index_2;
						data_manager_.set_index_value(index_id_[2], index_2);

						if (where_clause(2)) {
							for (int index_3 = index_restart[3];
									index_3 < upper_bound_[3]; ++index_3) {
								index_values_[3] = index_3;
								data_manager_.set_index_value(index_id_[3],
										index_3);

								if (where_clause(3)) {
									for (int index_4 = index_restart[4];
											index_4 < upper_bound_[4];
											++index_4) {
										index_values_[4] = index_4;
										data_manager_.set_index_value(
												index_id_[4], index_4);

										if (where_clause(4)) {
											for (int index_5 = index_restart[5];
													index_5 < upper_bound_[5];
													++index_5) {
												index_values_[5] = index_5;
												data_manager_.set_index_value(
														index_id_[5], index_5);

												if (where_clause(5)) {
													iteration_++;
													if ((iteration_ - 1)
															% num_workers_
															== company_rank_) {
														return true;
													}
												} // where 5
											} // index_5
											for (int i = 5; i < num_indices_;
													++i) {
												index_restart[i] =
														lower_seg_[i];
											}
										} // where 4
									} // index_4
									for (int i = 4; i < num_indices_; ++i) {
										index_restart[i] = lower_seg_[i];
									}
								} // where 3
							} // index_3
							for (int i = 3; i < num_indices_; ++i) {
								index_restart[i] = lower_seg_[i];
							}
						} // where 2
					} // index_2
					for (int i = 2; i < num_indices_; ++i) {
						index_restart[i] = lower_seg_[i];
					}
				} // where 1
			} // index_j
			for (int i = 1; i < num_indices_; ++i) {
				index_restart[i] = lower_seg_[i];
			}
		} // index_i
	}

	for (int i = 0; i < num_indices_; ++i) {
		index_restart[i] = index_values_[i];
	}

	for (int index_i = index_restart[0]; index_i < upper_bound_[0]; ++index_i) {
		index_values_[0] = index_i;
		data_manager_.set_index_value(index_id_[0], index_i);

		for (int index_j = index_restart[1]; index_j < upper_bound_[1];
				++index_j) {
			index_values_[1] = index_j;
			data_manager_.set_index_value(index_id_[1], index_j);

			if (where_clause(1)) {
				for (int index_2 = index_restart[2]; index_2 < upper_bound_[2];
						++index_2) {
					index_values_[2] = index_2;
					data_manager_.set_index_value(index_id_[2], index_2);

					if (where_clause(2)) {
						for (int index_3 = index_restart[3];
								index_3 < upper_bound_[3]; ++index_3) {
							index_values_[3] = index_3;
							data_manager_.set_index_value(index_id_[3],
									index_3);

							if (where_clause(3)) {
								for (int index_4 = index_restart[4];
										index_4 < upper_bound_[4]; ++index_4) {
									index_values_[4] = index_4;
									data_manager_.set_index_value(index_id_[4],
											index_4);

									if (where_clause(4)) {
										for (int index_5 = index_restart[5];
												index_5 < upper_bound_[5];
												++index_5) {
											index_values_[5] = index_5;
											data_manager_.set_index_value(
													index_id_[5], index_5);

											if (where_clause(5)) {
												if (loop_count > iteration_) {
													iteration_++;
													if ((iteration_ - 1)
															% num_workers_
															== company_rank_) {
														return true;
													}
												}
												++loop_count;
											} // where 5
										} // index_5
										for (int i = 5; i < num_indices_; ++i) {
											index_restart[i] = lower_seg_[i];
										}
									} // where 4
								} // index_4
								for (int i = 4; i < num_indices_; ++i) {
									index_restart[i] = lower_seg_[i];
								}
							} // where 3
						} // index_3
						for (int i = 3; i < num_indices_; ++i) {
							index_restart[i] = lower_seg_[i];
						}
					} // where 2
				} // index_2
				for (int i = 2; i < num_indices_; ++i) {
					index_restart[i] = lower_seg_[i];
				}
			} // where 1
		} // index_j
		for (int i = 1; i < num_indices_; ++i) {
			index_restart[i] = lower_seg_[i];
		}
	} // index_i

	return false; //this should be false here
}

Fragment_Rij_pp_pp_PardoLoopManager::Fragment_Rij_pp_pp_PardoLoopManager(
		int num_indices, const int (&index_id)[MAX_RANK],
		DataManager & data_manager, const SipTables & sip_tables,
		SIPMPIAttr & sip_mpi_attr, int num_where_clauses,
		Interpreter* interpreter, long& iteration) :
		FragmentPardoLoopManager(num_indices, index_id, data_manager,
				sip_tables), first_time_(true), iteration_(iteration), sip_mpi_attr_(
				sip_mpi_attr), num_where_clauses_(num_where_clauses), company_rank_(
				sip_mpi_attr.company_rank()), num_workers_(
				sip_mpi_attr_.num_workers()), interpreter_(interpreter) {

	form_rcut_dist();
	form_swmoa_frag();
}

Fragment_Rij_pp_pp_PardoLoopManager::~Fragment_Rij_pp_pp_PardoLoopManager() {
}

/*!
 -------------------------------------------
 _NRij_pp_pp_
 -------------------------------------------
 */

/*
 for each new special fragment where clause pattern, this should be the only thing realy changed.
 see comment above for fragment_special_where_clause syntax
 */
bool Fragment_NRij_pp_pp_PardoLoopManager::where_clause(int index) {
	bool where_;
	int ifrag = 0;
	int jfrag = 1;
	int ao = 1;
	int occ = 2;
	int virt = 3;
	int elst = 4;
	int rcut = 5;
	int NE = 0;
	int mo = 6;

	switch (index) {
	case 0:
	case 1:
		where_ = fragment_special_where_clause(rcut, jfrag, ifrag)
				&& fragment_special_where_clause(NE, jfrag, ifrag);
		break;
	case 2:
		where_ = fragment_special_where_clause(mo, index, ifrag);
		break;
	case 3:
		where_ = fragment_special_where_clause(mo, index, ifrag);
		break;
	case 4:
		where_ = fragment_special_where_clause(mo, index, jfrag);
		break;
	case 5:
		where_ = fragment_special_where_clause(mo, index, jfrag);
		break;
	default:
		where_ = false;
	}
	return where_;
}

bool Fragment_NRij_pp_pp_PardoLoopManager::do_update() {
	if (to_exit_)
		return false;
	bool more_iters;
	bool where_clauses_value;

	interpreter_->skip_where_clauses(num_where_clauses_);

	int loop_count = iteration_;
	int index_restart[MAX_RANK];

	if (first_time_) {
		first_time_ = false;
		more_iters = initialize_indices();

		for (int i = 0; i < num_indices_; ++i) {
			index_restart[i] = index_values_[i];
		}

		for (int index_i = index_restart[0]; index_i < upper_bound_[0];
				++index_i) {
			index_values_[0] = index_i;
			data_manager_.set_index_value(index_id_[0], index_i);

			for (int index_j = index_restart[1]; index_j < upper_bound_[1];
					++index_j) {
				index_values_[1] = index_j;
				data_manager_.set_index_value(index_id_[1], index_j);

				if (where_clause(1)) {
					for (int index_2 = index_restart[2];
							index_2 < upper_bound_[2]; ++index_2) {
						index_values_[2] = index_2;
						data_manager_.set_index_value(index_id_[2], index_2);

						if (where_clause(2)) {
							for (int index_3 = index_restart[3];
									index_3 < upper_bound_[3]; ++index_3) {
								index_values_[3] = index_3;
								data_manager_.set_index_value(index_id_[3],
										index_3);

								if (where_clause(3)) {
									for (int index_4 = index_restart[4];
											index_4 < upper_bound_[4];
											++index_4) {
										index_values_[4] = index_4;
										data_manager_.set_index_value(
												index_id_[4], index_4);

										if (where_clause(4)) {
											for (int index_5 = index_restart[5];
													index_5 < upper_bound_[5];
													++index_5) {
												index_values_[5] = index_5;
												data_manager_.set_index_value(
														index_id_[5], index_5);

												if (where_clause(5)) {
													iteration_++;
													if ((iteration_ - 1)
															% num_workers_
															== company_rank_) {
														return true;
													}
												} // where 5
											} // index_5
											for (int i = 5; i < num_indices_;
													++i) {
												index_restart[i] =
														lower_seg_[i];
											}
										} // where 4
									} // index_4
									for (int i = 4; i < num_indices_; ++i) {
										index_restart[i] = lower_seg_[i];
									}
								} // where 3
							} // index_3
							for (int i = 3; i < num_indices_; ++i) {
								index_restart[i] = lower_seg_[i];
							}
						} // where 2
					} // index_2
					for (int i = 2; i < num_indices_; ++i) {
						index_restart[i] = lower_seg_[i];
					}
				} // where 1
			} // index_j
			for (int i = 1; i < num_indices_; ++i) {
				index_restart[i] = lower_seg_[i];
			}
		} // index_i
	}

	for (int i = 0; i < num_indices_; ++i) {
		index_restart[i] = index_values_[i];
	}

	for (int index_i = index_restart[0]; index_i < upper_bound_[0]; ++index_i) {
		index_values_[0] = index_i;
		data_manager_.set_index_value(index_id_[0], index_i);

		for (int index_j = index_restart[1]; index_j < upper_bound_[1];
				++index_j) {
			index_values_[1] = index_j;
			data_manager_.set_index_value(index_id_[1], index_j);

			if (where_clause(1)) {
				for (int index_2 = index_restart[2]; index_2 < upper_bound_[2];
						++index_2) {
					index_values_[2] = index_2;
					data_manager_.set_index_value(index_id_[2], index_2);

					if (where_clause(2)) {
						for (int index_3 = index_restart[3];
								index_3 < upper_bound_[3]; ++index_3) {
							index_values_[3] = index_3;
							data_manager_.set_index_value(index_id_[3],
									index_3);

							if (where_clause(3)) {
								for (int index_4 = index_restart[4];
										index_4 < upper_bound_[4]; ++index_4) {
									index_values_[4] = index_4;
									data_manager_.set_index_value(index_id_[4],
											index_4);

									if (where_clause(4)) {
										for (int index_5 = index_restart[5];
												index_5 < upper_bound_[5];
												++index_5) {
											index_values_[5] = index_5;
											data_manager_.set_index_value(
													index_id_[5], index_5);

											if (where_clause(5)) {
												if (loop_count > iteration_) {
													iteration_++;
													if ((iteration_ - 1)
															% num_workers_
															== company_rank_) {
														return true;
													}
												}
												++loop_count;
											} // where 5
										} // index_5
										for (int i = 5; i < num_indices_; ++i) {
											index_restart[i] = lower_seg_[i];
										}
									} // where 4
								} // index_4
								for (int i = 4; i < num_indices_; ++i) {
									index_restart[i] = lower_seg_[i];
								}
							} // where 3
						} // index_3
						for (int i = 3; i < num_indices_; ++i) {
							index_restart[i] = lower_seg_[i];
						}
					} // where 2
				} // index_2
				for (int i = 2; i < num_indices_; ++i) {
					index_restart[i] = lower_seg_[i];
				}
			} // where 1
		} // index_j
		for (int i = 1; i < num_indices_; ++i) {
			index_restart[i] = lower_seg_[i];
		}
	} // index_i

	return false; //this should be false here
}

Fragment_NRij_pp_pp_PardoLoopManager::Fragment_NRij_pp_pp_PardoLoopManager(
		int num_indices, const int (&index_id)[MAX_RANK],
		DataManager & data_manager, const SipTables & sip_tables,
		SIPMPIAttr & sip_mpi_attr, int num_where_clauses,
		Interpreter* interpreter, long& iteration) :
		FragmentPardoLoopManager(num_indices, index_id, data_manager,
				sip_tables), first_time_(true), iteration_(iteration), sip_mpi_attr_(
				sip_mpi_attr), num_where_clauses_(num_where_clauses), company_rank_(
				sip_mpi_attr.company_rank()), num_workers_(
				sip_mpi_attr_.num_workers()), interpreter_(interpreter) {

	form_rcut_dist();
	form_swmoa_frag();
}

Fragment_NRij_pp_pp_PardoLoopManager::~Fragment_NRij_pp_pp_PardoLoopManager() {
}

} /* namespace sip */

#endif
