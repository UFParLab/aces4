/*
 * job_control.h
 *
 *  Created on: Nov 7, 2015
 *      Author: basbas
 */

#ifndef JOB_CONTROL_H_
#define JOB_CONTROL_H_

#include <ctime>
#include <string>

#ifdef HAVE_MPI
#include <mpi.h>
#endif

namespace sip {

class JobControl {


public:

	static JobControl* global;  //the globally accessible object.

	static void set_global_job_control(JobControl* job_control){
		if (global != NULL) delete global;
		global = job_control;
	}

	static const size_t default_max_worker_data_memory_usage = 2147483648; // Default 2GB
	static const size_t default_max_server_data_memory_usage = 2147483648; // Default 2GB

	/** Create a jobid for this job using the time.  This is a collective operation
	 * which ensures that all processes have the same id.
	 * @return new job_id.
	 */
	static std::string make_job_id();

	explicit JobControl(std::string job_id);

	JobControl(std::string job_id, std::string restart_id, int prog_num);

	JobControl(std::string job_id,
			std::size_t max_worker_data_memory_usage,
			std::size_t max_server_data_memory_usage);

	JobControl(std::string job_id, std::string restart_id, int prog_num,
			std::size_t max_worker_data_memory_usage,
			std::size_t max_server_data_memory_usage);

	~JobControl();

	void increment_program() {prog_num_++;};

	int get_program_num() { return prog_num_; };
	void set_program_num(int num) { prog_num_ = num; }

	void set_program_name(std::string s) { prog_name_ = s; }
	std::string get_program_name() { return prog_name_; }

	void reset_program_count() { prog_num_ = 0; }

	void set_max_worker_data_memory_usage(std::size_t m) {
		max_worker_data_memory_usage_ = m;
	}
	void set_max_server_data_memory_usage(std::size_t m) {
		max_server_data_memory_usage_ = m;
	}

	std::size_t get_max_worker_data_memory_usage() {  return max_worker_data_memory_usage_; }
	std::size_t get_max_server_data_memory_usage() {  return max_server_data_memory_usage_; }





	/**
	 * Set the job id to the given value
	 *
	 * This is used in testing.  Normally, the job id should be calculated from the
	 * time stamp using the parameterless version of this method.
	 *
	 * @param the_job_id
	 * @return
	 */
	std::string set_job_id(std::string the_job_id){
		job_id_ = the_job_id;
		return job_id_;
	}

	std::string get_job_id(){return job_id_;}

	 void set_restart_id(std::string the_restart_id){
		restart_id_ = the_restart_id;
	}
	std::string get_restart_id(){return restart_id_;}

	friend std::ostream& operator <<(std::ostream&, const JobControl&);


private:
	std::size_t max_worker_data_memory_usage_;
	std::size_t max_server_data_memory_usage_;
	std::string prog_name_;
	std::string job_id_;
	std::string restart_id_;
	int prog_num_;
};

} /* namespace sip */


#endif /* JOB_CONTROL_H_ */
