/*
 * loop_manager.h
 *
 *  Created on: Aug 23, 2013
 *      Author: basbas
 */

#ifndef FRAGMENT_LOOP_MANAGER_H_
#define FRAGMENT_LOOP_MANAGER_H_

#include "loop_manager.h"
#include "config.h"
#include "aces_defs.h"
#include "sip.h"
#include "array_constants.h"
#include "data_manager.h"

#include "sip_mpi_attr.h"

namespace sip {

class Interpreter;

#ifdef HAVE_MPI

/**
 * Special Loop manager for Fragment code mcpt2_corr_lowmem.
 * Optimizes where clause lookups that involves looking up blocks.
 *
 *  naming scheme: _<simple index fragment indices>_<occ (o), virt (v), ao (a)
 *  in ifrag>_<o,v,a in jfrag>
 *
 *  */

class Fragment_Nij_aa__PardoLoopManager: public LoopManager{
public:
	Fragment_Nij_aa__PardoLoopManager(int num_indices,
			const int (&index_ids)[MAX_RANK], DataManager & data_manager,
			const SipTables & sip_tables, SIPMPIAttr& sip_mpi_attr,
			int num_where_clauses, Interpreter* interpreter, long& iteration);
	virtual ~Fragment_Nij_aa__PardoLoopManager();
	friend std::ostream& operator<<(std::ostream&,
				const Fragment_Nij_aa__PardoLoopManager &);
private:
	virtual std::string to_string() const;
	virtual bool do_update();
	virtual void do_finalize();
	bool first_time_;
	int num_indices_;
	long& iteration_;
	index_selector_t index_id_;
	index_value_array_t lower_seg_;
	index_value_array_t upper_bound_;
	int num_where_clauses_;

	DataManager & data_manager_;
	const SipTables & sip_tables_;
	SIPMPIAttr & sip_mpi_attr_;
	int company_rank_;
	int num_workers_;
	Interpreter* interpreter_;

	int index_values_[MAX_RANK];
    
    bool initialize_indices();
    bool increment_simple_pair();
    bool increment_single_index(int index);
    bool increment_all();
    
    //double return_val_rcut_dist(int index1, int index2);
    //double return_val_elst_dist(int index1, int index2);
    //double return_val_swao_frag(int index1);
    //double return_val_swocca_frag(int index1);
    //double return_val_swvirta_frag(int index1);
    
    void form_rcut_dist();
    void form_elst_dist();
    void form_swao_frag();
    void form_swocca_frag();
    void form_swvirta_frag();
    
    bool fragment_special_where_clause(int typ, int index, int frag);
    bool where_clause(int index);
    
    std::vector< std::vector<int> > elst_dist;
    std::vector< std::vector<int> > rcut_dist;
    std::vector<int> swao_frag;
    std::vector<int> swocca_frag;
    std::vector<int> swvirta_frag;
};

class Fragment_Nij_a_a_PardoLoopManager: public LoopManager{
public:
	Fragment_Nij_a_a_PardoLoopManager(int num_indices,
			const int (&index_ids)[MAX_RANK], DataManager & data_manager,
			const SipTables & sip_tables, SIPMPIAttr& sip_mpi_attr,
			int num_where_clauses, Interpreter* interpreter, long& iteration);
	virtual ~Fragment_Nij_a_a_PardoLoopManager();
	friend std::ostream& operator<<(std::ostream&,
				const Fragment_Nij_a_a_PardoLoopManager &);
private:
	virtual std::string to_string() const;
	virtual bool do_update();
	virtual void do_finalize();
	bool first_time_;
	int num_indices_;
	long& iteration_;
	index_selector_t index_id_;
	index_value_array_t lower_seg_;
	index_value_array_t upper_bound_;
	int num_where_clauses_;

	DataManager & data_manager_;
	const SipTables & sip_tables_;
	SIPMPIAttr & sip_mpi_attr_;
	int company_rank_;
	int num_workers_;
	Interpreter* interpreter_;

	int index_values_[MAX_RANK];
    
    bool initialize_indices();
    bool increment_simple_pair();
    bool increment_single_index(int index);
    bool increment_all();
    
    //double return_val_rcut_dist(int index1, int index2);
    //double return_val_elst_dist(int index1, int index2);
    //double return_val_swao_frag(int index1);
    //double return_val_swocca_frag(int index1);
    //double return_val_swvirta_frag(int index1);
    
    void form_rcut_dist();
    void form_elst_dist();
    void form_swao_frag();
    void form_swocca_frag();
    void form_swvirta_frag();
    
    bool fragment_special_where_clause(int typ, int index, int frag);
    bool where_clause(int index);
    
    std::vector< std::vector<int> > elst_dist;
    std::vector< std::vector<int> > rcut_dist;
    std::vector<int> swao_frag;
    std::vector<int> swocca_frag;
    std::vector<int> swvirta_frag;
};

class Fragment_ij_ao_vo_PardoLoopManager: public LoopManager{
public:
	Fragment_ij_ao_vo_PardoLoopManager(int num_indices,
			const int (&index_ids)[MAX_RANK], DataManager & data_manager,
			const SipTables & sip_tables, SIPMPIAttr& sip_mpi_attr,
			int num_where_clauses, Interpreter* interpreter, long& iteration);
	virtual ~Fragment_ij_ao_vo_PardoLoopManager();
	friend std::ostream& operator<<(std::ostream&,
				const Fragment_ij_ao_vo_PardoLoopManager &);
private:
	virtual std::string to_string() const;
	virtual bool do_update();
	virtual void do_finalize();
	bool first_time_;
	int num_indices_;
	long& iteration_;
	index_selector_t index_id_;
	index_value_array_t lower_seg_;
	index_value_array_t upper_bound_;
	int num_where_clauses_;

	DataManager & data_manager_;
	const SipTables & sip_tables_;
	SIPMPIAttr & sip_mpi_attr_;
	int company_rank_;
	int num_workers_;
	Interpreter* interpreter_;

	int index_values_[MAX_RANK];

    bool initialize_indices();
	bool increment_simple_pair();
    bool increment_single_index(int index);
    bool increment_all();

    //double return_val_rcut_dist(int index1, int index2);
	//double return_val_elst_dist(int index1, int index2);
	//double return_val_swao_frag(int index1);
	//double return_val_swocca_frag(int index1);
	//double return_val_swvirta_frag(int index1);
    
    void form_rcut_dist();
    void form_elst_dist();
    void form_swao_frag();
    void form_swocca_frag();
    void form_swvirta_frag();
    
    bool fragment_special_where_clause(int typ, int index, int frag);
    bool where_clause(int index);
    
    std::vector< std::vector<int> > elst_dist;
    std::vector< std::vector<int> > rcut_dist;
    std::vector<int> swao_frag;
    std::vector<int> swocca_frag;
    std::vector<int> swvirta_frag;
    
};
    
    class Fragment_i_aa__PardoLoopManager: public LoopManager{
    public:
        Fragment_i_aa__PardoLoopManager(int num_indices,
                                           const int (&index_ids)[MAX_RANK], DataManager & data_manager,
                                           const SipTables & sip_tables, SIPMPIAttr& sip_mpi_attr,
                                           int num_where_clauses, Interpreter* interpreter, long& iteration);
        virtual ~Fragment_i_aa__PardoLoopManager();
        friend std::ostream& operator<<(std::ostream&,
                                        const Fragment_i_aa__PardoLoopManager &);
    private:
        virtual std::string to_string() const;
        virtual bool do_update();
        virtual void do_finalize();
        bool first_time_;
        int num_indices_;
        long& iteration_;
        index_selector_t index_id_;
        index_value_array_t lower_seg_;
        index_value_array_t upper_bound_;
        int num_where_clauses_;
        
        DataManager & data_manager_;
        const SipTables & sip_tables_;
        SIPMPIAttr & sip_mpi_attr_;
        int company_rank_;
        int num_workers_;
        Interpreter* interpreter_;
        
        int index_values_[MAX_RANK];
        
        bool initialize_indices();
        bool increment_simple_pair();
        bool increment_single_index(int index);
        bool increment_all();
        
        //double return_val_rcut_dist(int index1, int index2);
        //double return_val_elst_dist(int index1, int index2);
        //double return_val_swao_frag(int index1);
        //double return_val_swocca_frag(int index1);
        //double return_val_swvirta_frag(int index1);
        
        void form_rcut_dist();
        void form_elst_dist();
        void form_swao_frag();
        void form_swocca_frag();
        void form_swvirta_frag();
        
        bool fragment_special_where_clause(int typ, int index, int frag);
        bool where_clause(int index);
        
        std::vector< std::vector<int> > elst_dist;
        std::vector< std::vector<int> > rcut_dist;
        std::vector<int> swao_frag;
        std::vector<int> swocca_frag;
        std::vector<int> swvirta_frag;
        
    };
    
    class Fragment_i_vo__PardoLoopManager: public LoopManager{
    public:
        Fragment_i_vo__PardoLoopManager(int num_indices,
                                           const int (&index_ids)[MAX_RANK], DataManager & data_manager,
                                           const SipTables & sip_tables, SIPMPIAttr& sip_mpi_attr,
                                           int num_where_clauses, Interpreter* interpreter, long& iteration);
        virtual ~Fragment_i_vo__PardoLoopManager();
        friend std::ostream& operator<<(std::ostream&,
                                        const Fragment_i_vo__PardoLoopManager &);
    private:
        virtual std::string to_string() const;
        virtual bool do_update();
        virtual void do_finalize();
        bool first_time_;
        int num_indices_;
        long& iteration_;
        index_selector_t index_id_;
        index_value_array_t lower_seg_;
        index_value_array_t upper_bound_;
        int num_where_clauses_;
        
        DataManager & data_manager_;
        const SipTables & sip_tables_;
        SIPMPIAttr & sip_mpi_attr_;
        int company_rank_;
        int num_workers_;
        Interpreter* interpreter_;
        
        int index_values_[MAX_RANK];
        
        bool initialize_indices();
        bool increment_simple_pair();
        bool increment_single_index(int index);
        bool increment_all();
        
        //double return_val_rcut_dist(int index1, int index2);
        //double return_val_elst_dist(int index1, int index2);
        //double return_val_swao_frag(int index1);
        //double return_val_swocca_frag(int index1);
        //double return_val_swvirta_frag(int index1);
        
        void form_rcut_dist();
        void form_elst_dist();
        void form_swao_frag();
        void form_swocca_frag();
        void form_swvirta_frag();
        
        bool fragment_special_where_clause(int typ, int index, int frag);
        bool where_clause(int index);
        
        std::vector< std::vector<int> > elst_dist;
        std::vector< std::vector<int> > rcut_dist;
        std::vector<int> swao_frag;
        std::vector<int> swocca_frag;
        std::vector<int> swvirta_frag;
        
    };

    class Fragment_i_vovo__PardoLoopManager: public LoopManager{
    public:
        Fragment_i_vovo__PardoLoopManager(int num_indices,
                                           const int (&index_ids)[MAX_RANK], DataManager & data_manager,
                                           const SipTables & sip_tables, SIPMPIAttr& sip_mpi_attr,
                                           int num_where_clauses, Interpreter* interpreter, long& iteration);
        virtual ~Fragment_i_vovo__PardoLoopManager();
        friend std::ostream& operator<<(std::ostream&,
                                        const Fragment_i_vovo__PardoLoopManager &);
    private:
        virtual std::string to_string() const;
        virtual bool do_update();
        virtual void do_finalize();
        bool first_time_;
        int num_indices_;
        long& iteration_;
        index_selector_t index_id_;
        index_value_array_t lower_seg_;
        index_value_array_t upper_bound_;
        int num_where_clauses_;
        
        DataManager & data_manager_;
        const SipTables & sip_tables_;
        SIPMPIAttr & sip_mpi_attr_;
        int company_rank_;
        int num_workers_;
        Interpreter* interpreter_;
        
        int index_values_[MAX_RANK];
        
        bool initialize_indices();
        bool increment_simple_pair();
        bool increment_single_index(int index);
        bool increment_all();
        
        //double return_val_rcut_dist(int index1, int index2);
        //double return_val_elst_dist(int index1, int index2);
        //double return_val_swao_frag(int index1);
        //double return_val_swocca_frag(int index1);
        //double return_val_swvirta_frag(int index1);
        
        void form_rcut_dist();
        void form_elst_dist();
        void form_swao_frag();
        void form_swocca_frag();
        void form_swvirta_frag();
        
        bool fragment_special_where_clause(int typ, int index, int frag);
        bool where_clause(int index);
        
        std::vector< std::vector<int> > elst_dist;
        std::vector< std::vector<int> > rcut_dist;
        std::vector<int> swao_frag;
        std::vector<int> swocca_frag;
        std::vector<int> swvirta_frag;
        
    };

class Fragment_ij_aaa__PardoLoopManager: public LoopManager{
public:
	Fragment_ij_aaa__PardoLoopManager(int num_indices,
			const int (&index_ids)[MAX_RANK], DataManager & data_manager,
			const SipTables & sip_tables, SIPMPIAttr& sip_mpi_attr,
			int num_where_clauses, Interpreter* interpreter, long& iteration);
	virtual ~Fragment_ij_aaa__PardoLoopManager();
	friend std::ostream& operator<<(std::ostream&,
				const Fragment_ij_aaa__PardoLoopManager &);
private:
	virtual std::string to_string() const;
	virtual bool do_update();
	virtual void do_finalize();
	bool first_time_;
	int num_indices_;
	long& iteration_;
	index_selector_t index_id_;
	index_value_array_t lower_seg_;
	index_value_array_t upper_bound_;
	int num_where_clauses_;

	DataManager & data_manager_;
	const SipTables & sip_tables_;
	SIPMPIAttr & sip_mpi_attr_;
	int company_rank_;
	int num_workers_;
	Interpreter* interpreter_;

	int index_values_[MAX_RANK];

    bool initialize_indices();
	bool increment_simple_pair();
    bool increment_single_index(int index);
    bool increment_all();

    //double return_val_rcut_dist(int index1, int index2);
	//double return_val_elst_dist(int index1, int index2);
	//double return_val_swao_frag(int index1);
	//double return_val_swocca_frag(int index1);
	//double return_val_swvirta_frag(int index1);
    
    void form_rcut_dist();
    void form_elst_dist();
    void form_swao_frag();
    void form_swocca_frag();
    void form_swvirta_frag();
    
    bool fragment_special_where_clause(int typ, int index, int frag);
    bool where_clause(int index);
    
    std::vector< std::vector<int> > elst_dist;
    std::vector< std::vector<int> > rcut_dist;
    std::vector<int> swao_frag;
    std::vector<int> swocca_frag;
    std::vector<int> swvirta_frag;
    
};

class Fragment_ij_aa_a_PardoLoopManager: public LoopManager{
public:
	Fragment_ij_aa_a_PardoLoopManager(int num_indices,
			const int (&index_ids)[MAX_RANK], DataManager & data_manager,
			const SipTables & sip_tables, SIPMPIAttr& sip_mpi_attr,
			int num_where_clauses, Interpreter* interpreter, long& iteration);
	virtual ~Fragment_ij_aa_a_PardoLoopManager();
	friend std::ostream& operator<<(std::ostream&,
				const Fragment_ij_aa_a_PardoLoopManager &);
private:
	virtual std::string to_string() const;
	virtual bool do_update();
	virtual void do_finalize();
	bool first_time_;
	int num_indices_;
	long& iteration_;
	index_selector_t index_id_;
	index_value_array_t lower_seg_;
	index_value_array_t upper_bound_;
	int num_where_clauses_;

	DataManager & data_manager_;
	const SipTables & sip_tables_;
	SIPMPIAttr & sip_mpi_attr_;
	int company_rank_;
	int num_workers_;
	Interpreter* interpreter_;

	int index_values_[MAX_RANK];

    bool initialize_indices();
	bool increment_simple_pair();
    bool increment_single_index(int index);
    bool increment_all();

    //double return_val_rcut_dist(int index1, int index2);
	//double return_val_elst_dist(int index1, int index2);
	//double return_val_swao_frag(int index1);
	//double return_val_swocca_frag(int index1);
	//double return_val_swvirta_frag(int index1);
    
    void form_rcut_dist();
    void form_elst_dist();
    void form_swao_frag();
    void form_swocca_frag();
    void form_swvirta_frag();
    
    bool fragment_special_where_clause(int typ, int index, int frag);
    bool where_clause(int index);
    
    std::vector< std::vector<int> > elst_dist;
    std::vector< std::vector<int> > rcut_dist;
    std::vector<int> swao_frag;
    std::vector<int> swocca_frag;
    std::vector<int> swvirta_frag;
    
};

class Fragment_ij_ao_ao_PardoLoopManager: public LoopManager{
public:
	Fragment_ij_ao_ao_PardoLoopManager(int num_indices,
			const int (&index_ids)[MAX_RANK], DataManager & data_manager,
			const SipTables & sip_tables, SIPMPIAttr& sip_mpi_attr,
			int num_where_clauses, Interpreter* interpreter, long& iteration);
	virtual ~Fragment_ij_ao_ao_PardoLoopManager();
	friend std::ostream& operator<<(std::ostream&,
				const Fragment_ij_ao_ao_PardoLoopManager &);
private:
	virtual std::string to_string() const;
	virtual bool do_update();
	virtual void do_finalize();
	bool first_time_;
	int num_indices_;
	long& iteration_;
	index_selector_t index_id_;
	index_value_array_t lower_seg_;
	index_value_array_t upper_bound_;
	int num_where_clauses_;

	DataManager & data_manager_;
	const SipTables & sip_tables_;
	SIPMPIAttr & sip_mpi_attr_;
	int company_rank_;
	int num_workers_;
	Interpreter* interpreter_;

	int index_values_[MAX_RANK];

    bool initialize_indices();
	bool increment_simple_pair();
    bool increment_single_index(int index);
    bool increment_all();

    //double return_val_rcut_dist(int index1, int index2);
	//double return_val_elst_dist(int index1, int index2);
	//double return_val_swao_frag(int index1);
	//double return_val_swocca_frag(int index1);
	//double return_val_swvirta_frag(int index1);
    
    void form_rcut_dist();
    void form_elst_dist();
    void form_swao_frag();
    void form_swocca_frag();
    void form_swvirta_frag();
    
    bool fragment_special_where_clause(int typ, int index, int frag);
    bool where_clause(int index);
    
    std::vector< std::vector<int> > elst_dist;
    std::vector< std::vector<int> > rcut_dist;
    std::vector<int> swao_frag;
    std::vector<int> swocca_frag;
    std::vector<int> swvirta_frag;
    
};

class Fragment_ij_aa_vo_PardoLoopManager: public LoopManager{
public:
	Fragment_ij_aa_vo_PardoLoopManager(int num_indices,
			const int (&index_ids)[MAX_RANK], DataManager & data_manager,
			const SipTables & sip_tables, SIPMPIAttr& sip_mpi_attr,
			int num_where_clauses, Interpreter* interpreter, long& iteration);
	virtual ~Fragment_ij_aa_vo_PardoLoopManager();
	friend std::ostream& operator<<(std::ostream&,
				const Fragment_ij_aa_vo_PardoLoopManager &);
private:
	virtual std::string to_string() const;
	virtual bool do_update();
	virtual void do_finalize();
	bool first_time_;
	int num_indices_;
	long& iteration_;
	index_selector_t index_id_;
	index_value_array_t lower_seg_;
	index_value_array_t upper_bound_;
	int num_where_clauses_;

	DataManager & data_manager_;
	const SipTables & sip_tables_;
	SIPMPIAttr & sip_mpi_attr_;
	int company_rank_;
	int num_workers_;
	Interpreter* interpreter_;

	int index_values_[MAX_RANK];

    bool initialize_indices();
	bool increment_simple_pair();
    bool increment_single_index(int index);
    bool increment_all();

    //double return_val_rcut_dist(int index1, int index2);
	//double return_val_elst_dist(int index1, int index2);
	//double return_val_swao_frag(int index1);
	//double return_val_swocca_frag(int index1);
	//double return_val_swvirta_frag(int index1);
    
    void form_rcut_dist();
    void form_elst_dist();
    void form_swao_frag();
    void form_swocca_frag();
    void form_swvirta_frag();
    
    bool fragment_special_where_clause(int typ, int index, int frag);
    bool where_clause(int index);
    
    std::vector< std::vector<int> > elst_dist;
    std::vector< std::vector<int> > rcut_dist;
    std::vector<int> swao_frag;
    std::vector<int> swocca_frag;
    std::vector<int> swvirta_frag;
    
};

class Fragment_ij_aa_oo_PardoLoopManager: public LoopManager{
public:
	Fragment_ij_aa_oo_PardoLoopManager(int num_indices,
			const int (&index_ids)[MAX_RANK], DataManager & data_manager,
			const SipTables & sip_tables, SIPMPIAttr& sip_mpi_attr,
			int num_where_clauses, Interpreter* interpreter, long& iteration);
	virtual ~Fragment_ij_aa_oo_PardoLoopManager();
	friend std::ostream& operator<<(std::ostream&,
				const Fragment_ij_aa_oo_PardoLoopManager &);
private:
	virtual std::string to_string() const;
	virtual bool do_update();
	virtual void do_finalize();
	bool first_time_;
	int num_indices_;
	long& iteration_;
	index_selector_t index_id_;
	index_value_array_t lower_seg_;
	index_value_array_t upper_bound_;
	int num_where_clauses_;

	DataManager & data_manager_;
	const SipTables & sip_tables_;
	SIPMPIAttr & sip_mpi_attr_;
	int company_rank_;
	int num_workers_;
	Interpreter* interpreter_;

	int index_values_[MAX_RANK];

    bool initialize_indices();
	bool increment_simple_pair();
    bool increment_single_index(int index);
    bool increment_all();

    //double return_val_rcut_dist(int index1, int index2);
	//double return_val_elst_dist(int index1, int index2);
	//double return_val_swao_frag(int index1);
	//double return_val_swocca_frag(int index1);
	//double return_val_swvirta_frag(int index1);
    
    void form_rcut_dist();
    void form_elst_dist();
    void form_swao_frag();
    void form_swocca_frag();
    void form_swvirta_frag();
    
    bool fragment_special_where_clause(int typ, int index, int frag);
    bool where_clause(int index);
    
    std::vector< std::vector<int> > elst_dist;
    std::vector< std::vector<int> > rcut_dist;
    std::vector<int> swao_frag;
    std::vector<int> swocca_frag;
    std::vector<int> swvirta_frag;
    
};

class Fragment_i_aaoo__PardoLoopManager: public LoopManager{
public:
	Fragment_i_aaoo__PardoLoopManager(int num_indices,
			const int (&index_ids)[MAX_RANK], DataManager & data_manager,
			const SipTables & sip_tables, SIPMPIAttr& sip_mpi_attr,
			int num_where_clauses, Interpreter* interpreter, long& iteration);
	virtual ~Fragment_i_aaoo__PardoLoopManager();
	friend std::ostream& operator<<(std::ostream&,
				const Fragment_i_aaoo__PardoLoopManager &);
private:
	virtual std::string to_string() const;
	virtual bool do_update();
	virtual void do_finalize();
	bool first_time_;
	int num_indices_;
	long& iteration_;
	index_selector_t index_id_;
	index_value_array_t lower_seg_;
	index_value_array_t upper_bound_;
	int num_where_clauses_;

	DataManager & data_manager_;
	const SipTables & sip_tables_;
	SIPMPIAttr & sip_mpi_attr_;
	int company_rank_;
	int num_workers_;
	Interpreter* interpreter_;

	int index_values_[MAX_RANK];

    bool initialize_indices();
	bool increment_simple_pair();
    bool increment_single_index(int index);
    bool increment_all();

    //double return_val_rcut_dist(int index1, int index2);
	//double return_val_elst_dist(int index1, int index2);
	//double return_val_swao_frag(int index1);
	//double return_val_swocca_frag(int index1);
	//double return_val_swvirta_frag(int index1);
    
    void form_rcut_dist();
    void form_elst_dist();
    void form_swao_frag();
    void form_swocca_frag();
    void form_swvirta_frag();
    
    bool fragment_special_where_clause(int typ, int index, int frag);
    bool where_clause(int index);
    
    std::vector< std::vector<int> > elst_dist;
    std::vector< std::vector<int> > rcut_dist;
    std::vector<int> swao_frag;
    std::vector<int> swocca_frag;
    std::vector<int> swvirta_frag;
    
};


class Fragment_ij_aoa_o_PardoLoopManager: public LoopManager{
public:
	Fragment_ij_aoa_o_PardoLoopManager(int num_indices,
			const int (&index_ids)[MAX_RANK], DataManager & data_manager,
			const SipTables & sip_tables, SIPMPIAttr& sip_mpi_attr,
			int num_where_clauses, Interpreter* interpreter, long& iteration);
	virtual ~Fragment_ij_aoa_o_PardoLoopManager();
	friend std::ostream& operator<<(std::ostream&,
				const Fragment_ij_aoa_o_PardoLoopManager &);
private:
	virtual std::string to_string() const;
	virtual bool do_update();
	virtual void do_finalize();
	bool first_time_;
	int num_indices_;
	long& iteration_;
	index_selector_t index_id_;
	index_value_array_t lower_seg_;
	index_value_array_t upper_bound_;
	int num_where_clauses_;

	DataManager & data_manager_;
	const SipTables & sip_tables_;
	SIPMPIAttr & sip_mpi_attr_;
	int company_rank_;
	int num_workers_;
	Interpreter* interpreter_;

	int index_values_[MAX_RANK];

    bool initialize_indices();
	bool increment_simple_pair();
    bool increment_single_index(int index);
    bool increment_all();

    //double return_val_rcut_dist(int index1, int index2);
	//double return_val_elst_dist(int index1, int index2);
	//double return_val_swao_frag(int index1);
	//double return_val_swocca_frag(int index1);
	//double return_val_swvirta_frag(int index1);
    
    void form_rcut_dist();
    void form_elst_dist();
    void form_swao_frag();
    void form_swocca_frag();
    void form_swvirta_frag();
    
    bool fragment_special_where_clause(int typ, int index, int frag);
    bool where_clause(int index);
    
    std::vector< std::vector<int> > elst_dist;
    std::vector< std::vector<int> > rcut_dist;
    std::vector<int> swao_frag;
    std::vector<int> swocca_frag;
    std::vector<int> swvirta_frag;
    
};

class Fragment_ij_av_vo_PardoLoopManager: public LoopManager{
public:
	Fragment_ij_av_vo_PardoLoopManager(int num_indices,
			const int (&index_ids)[MAX_RANK], DataManager & data_manager,
			const SipTables & sip_tables, SIPMPIAttr& sip_mpi_attr,
			int num_where_clauses, Interpreter* interpreter, long& iteration);
	virtual ~Fragment_ij_av_vo_PardoLoopManager();
	friend std::ostream& operator<<(std::ostream&,
				const Fragment_ij_av_vo_PardoLoopManager &);
private:
	virtual std::string to_string() const;
	virtual bool do_update();
	virtual void do_finalize();
	bool first_time_;
	int num_indices_;
	long& iteration_;
	index_selector_t index_id_;
	index_value_array_t lower_seg_;
	index_value_array_t upper_bound_;
	int num_where_clauses_;

	DataManager & data_manager_;
	const SipTables & sip_tables_;
	SIPMPIAttr & sip_mpi_attr_;
	int company_rank_;
	int num_workers_;
	Interpreter* interpreter_;

	int index_values_[MAX_RANK];

    bool initialize_indices();
	bool increment_simple_pair();
    bool increment_single_index(int index);
    bool increment_all();

    //double return_val_rcut_dist(int index1, int index2);
	//double return_val_elst_dist(int index1, int index2);
	//double return_val_swao_frag(int index1);
	//double return_val_swocca_frag(int index1);
	//double return_val_swvirta_frag(int index1);
    
    void form_rcut_dist();
    void form_elst_dist();
    void form_swao_frag();
    void form_swocca_frag();
    void form_swvirta_frag();
    
    bool fragment_special_where_clause(int typ, int index, int frag);
    bool where_clause(int index);
    
    std::vector< std::vector<int> > elst_dist;
    std::vector< std::vector<int> > rcut_dist;
    std::vector<int> swao_frag;
    std::vector<int> swocca_frag;
    std::vector<int> swvirta_frag;
    
};

class Fragment_ij_av_oo_PardoLoopManager: public LoopManager{
public:
	Fragment_ij_av_oo_PardoLoopManager(int num_indices,
			const int (&index_ids)[MAX_RANK], DataManager & data_manager,
			const SipTables & sip_tables, SIPMPIAttr& sip_mpi_attr,
			int num_where_clauses, Interpreter* interpreter, long& iteration);
	virtual ~Fragment_ij_av_oo_PardoLoopManager();
	friend std::ostream& operator<<(std::ostream&,
				const Fragment_ij_av_oo_PardoLoopManager &);
private:
	virtual std::string to_string() const;
	virtual bool do_update();
	virtual void do_finalize();
	bool first_time_;
	int num_indices_;
	long& iteration_;
	index_selector_t index_id_;
	index_value_array_t lower_seg_;
	index_value_array_t upper_bound_;
	int num_where_clauses_;

	DataManager & data_manager_;
	const SipTables & sip_tables_;
	SIPMPIAttr & sip_mpi_attr_;
	int company_rank_;
	int num_workers_;
	Interpreter* interpreter_;

	int index_values_[MAX_RANK];

    bool initialize_indices();
	bool increment_simple_pair();
    bool increment_single_index(int index);
    bool increment_all();

    //double return_val_rcut_dist(int index1, int index2);
	//double return_val_elst_dist(int index1, int index2);
	//double return_val_swao_frag(int index1);
	//double return_val_swocca_frag(int index1);
	//double return_val_swvirta_frag(int index1);
    
    void form_rcut_dist();
    void form_elst_dist();
    void form_swao_frag();
    void form_swocca_frag();
    void form_swvirta_frag();
    
    bool fragment_special_where_clause(int typ, int index, int frag);
    bool where_clause(int index);
    
    std::vector< std::vector<int> > elst_dist;
    std::vector< std::vector<int> > rcut_dist;
    std::vector<int> swao_frag;
    std::vector<int> swocca_frag;
    std::vector<int> swvirta_frag;
    
};

class Fragment_ij_ao_oo_PardoLoopManager: public LoopManager{
public:
	Fragment_ij_ao_oo_PardoLoopManager(int num_indices,
			const int (&index_ids)[MAX_RANK], DataManager & data_manager,
			const SipTables & sip_tables, SIPMPIAttr& sip_mpi_attr,
			int num_where_clauses, Interpreter* interpreter, long& iteration);
	virtual ~Fragment_ij_ao_oo_PardoLoopManager();
	friend std::ostream& operator<<(std::ostream&,
				const Fragment_ij_ao_oo_PardoLoopManager &);
private:
	virtual std::string to_string() const;
	virtual bool do_update();
	virtual void do_finalize();
	bool first_time_;
	int num_indices_;
	long& iteration_;
	index_selector_t index_id_;
	index_value_array_t lower_seg_;
	index_value_array_t upper_bound_;
	int num_where_clauses_;

	DataManager & data_manager_;
	const SipTables & sip_tables_;
	SIPMPIAttr & sip_mpi_attr_;
	int company_rank_;
	int num_workers_;
	Interpreter* interpreter_;

	int index_values_[MAX_RANK];

    bool initialize_indices();
	bool increment_simple_pair();
    bool increment_single_index(int index);
    bool increment_all();

    //double return_val_rcut_dist(int index1, int index2);
	//double return_val_elst_dist(int index1, int index2);
	//double return_val_swao_frag(int index1);
	//double return_val_swocca_frag(int index1);
	//double return_val_swvirta_frag(int index1);
    
    void form_rcut_dist();
    void form_elst_dist();
    void form_swao_frag();
    void form_swocca_frag();
    void form_swvirta_frag();
    
    bool fragment_special_where_clause(int typ, int index, int frag);
    bool where_clause(int index);
    
    std::vector< std::vector<int> > elst_dist;
    std::vector< std::vector<int> > rcut_dist;
    std::vector<int> swao_frag;
    std::vector<int> swocca_frag;
    std::vector<int> swvirta_frag;
    
};

class Fragment_ij_oo_ao_PardoLoopManager: public LoopManager{
public:
	Fragment_ij_oo_ao_PardoLoopManager(int num_indices,
			const int (&index_ids)[MAX_RANK], DataManager & data_manager,
			const SipTables & sip_tables, SIPMPIAttr& sip_mpi_attr,
			int num_where_clauses, Interpreter* interpreter, long& iteration);
	virtual ~Fragment_ij_oo_ao_PardoLoopManager();
	friend std::ostream& operator<<(std::ostream&,
				const Fragment_ij_oo_ao_PardoLoopManager &);
private:
	virtual std::string to_string() const;
	virtual bool do_update();
	virtual void do_finalize();
	bool first_time_;
	int num_indices_;
	long& iteration_;
	index_selector_t index_id_;
	index_value_array_t lower_seg_;
	index_value_array_t upper_bound_;
	int num_where_clauses_;

	DataManager & data_manager_;
	const SipTables & sip_tables_;
	SIPMPIAttr & sip_mpi_attr_;
	int company_rank_;
	int num_workers_;
	Interpreter* interpreter_;

	int index_values_[MAX_RANK];

    bool initialize_indices();
	bool increment_simple_pair();
    bool increment_single_index(int index);
    bool increment_all();

    //double return_val_rcut_dist(int index1, int index2);
	//double return_val_elst_dist(int index1, int index2);
	//double return_val_swao_frag(int index1);
	//double return_val_swocca_frag(int index1);
	//double return_val_swvirta_frag(int index1);
    
    void form_rcut_dist();
    void form_elst_dist();
    void form_swao_frag();
    void form_swocca_frag();
    void form_swvirta_frag();
    
    bool fragment_special_where_clause(int typ, int index, int frag);
    bool where_clause(int index);
    
    std::vector< std::vector<int> > elst_dist;
    std::vector< std::vector<int> > rcut_dist;
    std::vector<int> swao_frag;
    std::vector<int> swocca_frag;
    std::vector<int> swvirta_frag;
    
};

class Fragment_ij_aoo_o_PardoLoopManager: public LoopManager{
public:
	Fragment_ij_aoo_o_PardoLoopManager(int num_indices,
			const int (&index_ids)[MAX_RANK], DataManager & data_manager,
			const SipTables & sip_tables, SIPMPIAttr& sip_mpi_attr,
			int num_where_clauses, Interpreter* interpreter, long& iteration);
	virtual ~Fragment_ij_aoo_o_PardoLoopManager();
	friend std::ostream& operator<<(std::ostream&,
				const Fragment_ij_aoo_o_PardoLoopManager &);
private:
	virtual std::string to_string() const;
	virtual bool do_update();
	virtual void do_finalize();
	bool first_time_;
	int num_indices_;
	long& iteration_;
	index_selector_t index_id_;
	index_value_array_t lower_seg_;
	index_value_array_t upper_bound_;
	int num_where_clauses_;

	DataManager & data_manager_;
	const SipTables & sip_tables_;
	SIPMPIAttr & sip_mpi_attr_;
	int company_rank_;
	int num_workers_;
	Interpreter* interpreter_;

	int index_values_[MAX_RANK];

    bool initialize_indices();
	bool increment_simple_pair();
    bool increment_single_index(int index);
    bool increment_all();

    //double return_val_rcut_dist(int index1, int index2);
	//double return_val_elst_dist(int index1, int index2);
	//double return_val_swao_frag(int index1);
	//double return_val_swocca_frag(int index1);
	//double return_val_swvirta_frag(int index1);
    
    void form_rcut_dist();
    void form_elst_dist();
    void form_swao_frag();
    void form_swocca_frag();
    void form_swvirta_frag();
    
    bool fragment_special_where_clause(int typ, int index, int frag);
    bool where_clause(int index);
    
    std::vector< std::vector<int> > elst_dist;
    std::vector< std::vector<int> > rcut_dist;
    std::vector<int> swao_frag;
    std::vector<int> swocca_frag;
    std::vector<int> swvirta_frag;
    
};
    
class Fragment_ij_vo_vo_PardoLoopManager: public LoopManager{
public:
	Fragment_ij_vo_vo_PardoLoopManager(int num_indices,
			const int (&index_ids)[MAX_RANK], DataManager & data_manager,
			const SipTables & sip_tables, SIPMPIAttr& sip_mpi_attr,
			int num_where_clauses, Interpreter* interpreter, long& iteration);
	virtual ~Fragment_ij_vo_vo_PardoLoopManager();
	friend std::ostream& operator<<(std::ostream&,
				const Fragment_ij_vo_vo_PardoLoopManager &);
private:
	virtual std::string to_string() const;
	virtual bool do_update();
	virtual void do_finalize();
	bool first_time_;
	int num_indices_;
	long& iteration_;
	index_selector_t index_id_;
	index_value_array_t lower_seg_;
	index_value_array_t upper_bound_;
	int num_where_clauses_;

	DataManager & data_manager_;
	const SipTables & sip_tables_;
	SIPMPIAttr & sip_mpi_attr_;
	int company_rank_;
	int num_workers_;
	Interpreter* interpreter_;

	int index_values_[MAX_RANK];

    bool initialize_indices();
	bool increment_simple_pair();
    bool increment_single_index(int index);
    bool increment_all();

    //double return_val_rcut_dist(int index1, int index2);
	//double return_val_elst_dist(int index1, int index2);
	//double return_val_swao_frag(int index1);
	//double return_val_swocca_frag(int index1);
	//double return_val_swvirta_frag(int index1);
    
    void form_rcut_dist();
    void form_elst_dist();
    void form_swao_frag();
    void form_swocca_frag();
    void form_swvirta_frag();
    
    bool fragment_special_where_clause(int typ, int index, int frag);
    bool where_clause(int index);
    
    std::vector< std::vector<int> > elst_dist;
    std::vector< std::vector<int> > rcut_dist;
    std::vector<int> swao_frag;
    std::vector<int> swocca_frag;
    std::vector<int> swvirta_frag;
    
};

    
class Fragment_Nij_vo_vo_PardoLoopManager: public LoopManager{
public:
	Fragment_Nij_vo_vo_PardoLoopManager(int num_indices,
			const int (&index_ids)[MAX_RANK], DataManager & data_manager,
			const SipTables & sip_tables, SIPMPIAttr& sip_mpi_attr,
			int num_where_clauses, Interpreter* interpreter, long& iteration);
	virtual ~Fragment_Nij_vo_vo_PardoLoopManager();
	friend std::ostream& operator<<(std::ostream&,
				const Fragment_Nij_vo_vo_PardoLoopManager &);
private:
	virtual std::string to_string() const;
	virtual bool do_update();
	virtual void do_finalize();
	bool first_time_;
	int num_indices_;
	long& iteration_;
	index_selector_t index_id_;
	index_value_array_t lower_seg_;
	index_value_array_t upper_bound_;
	int num_where_clauses_;

	DataManager & data_manager_;
	const SipTables & sip_tables_;
	SIPMPIAttr & sip_mpi_attr_;
	int company_rank_;
	int num_workers_;
	Interpreter* interpreter_;

	int index_values_[MAX_RANK];

    bool initialize_indices();
	bool increment_simple_pair();
    bool increment_single_index(int index);
    bool increment_all();

    //double return_val_rcut_dist(int index1, int index2);
	//double return_val_elst_dist(int index1, int index2);
	//double return_val_swao_frag(int index1);
	//double return_val_swocca_frag(int index1);
	//double return_val_swvirta_frag(int index1);
    
    void form_rcut_dist();
    void form_elst_dist();
    void form_swao_frag();
    void form_swocca_frag();
    void form_swvirta_frag();
    
    bool fragment_special_where_clause(int typ, int index, int frag);
    bool where_clause(int index);
    
    std::vector< std::vector<int> > elst_dist;
    std::vector< std::vector<int> > rcut_dist;
    std::vector<int> swao_frag;
    std::vector<int> swocca_frag;
    std::vector<int> swvirta_frag;
    
};

    
class Fragment_NRij_vo_vo_PardoLoopManager: public LoopManager{
public:
	Fragment_NRij_vo_vo_PardoLoopManager(int num_indices,
			const int (&index_ids)[MAX_RANK], DataManager & data_manager,
			const SipTables & sip_tables, SIPMPIAttr& sip_mpi_attr,
			int num_where_clauses, Interpreter* interpreter, long& iteration);
	virtual ~Fragment_NRij_vo_vo_PardoLoopManager();
	friend std::ostream& operator<<(std::ostream&,
				const Fragment_NRij_vo_vo_PardoLoopManager &);
private:
	virtual std::string to_string() const;
	virtual bool do_update();
	virtual void do_finalize();
	bool first_time_;
	int num_indices_;
	long& iteration_;
	index_selector_t index_id_;
	index_value_array_t lower_seg_;
	index_value_array_t upper_bound_;
	int num_where_clauses_;

	DataManager & data_manager_;
	const SipTables & sip_tables_;
	SIPMPIAttr & sip_mpi_attr_;
	int company_rank_;
	int num_workers_;
	Interpreter* interpreter_;

	int index_values_[MAX_RANK];

    bool initialize_indices();
	bool increment_simple_pair();
    bool increment_single_index(int index);
    bool increment_all();

    //double return_val_rcut_dist(int index1, int index2);
	//double return_val_elst_dist(int index1, int index2);
	//double return_val_swao_frag(int index1);
	//double return_val_swocca_frag(int index1);
	//double return_val_swvirta_frag(int index1);
    
    void form_rcut_dist();
    void form_elst_dist();
    void form_swao_frag();
    void form_swocca_frag();
    void form_swvirta_frag();
    
    bool fragment_special_where_clause(int typ, int index, int frag);
    bool where_clause(int index);
    
    std::vector< std::vector<int> > elst_dist;
    std::vector< std::vector<int> > rcut_dist;
    std::vector<int> swao_frag;
    std::vector<int> swocca_frag;
    std::vector<int> swvirta_frag;
    
};

    
class Fragment_Rij_vo_vo_PardoLoopManager: public LoopManager{
public:
	Fragment_Rij_vo_vo_PardoLoopManager(int num_indices,
			const int (&index_ids)[MAX_RANK], DataManager & data_manager,
			const SipTables & sip_tables, SIPMPIAttr& sip_mpi_attr,
			int num_where_clauses, Interpreter* interpreter, long& iteration);
	virtual ~Fragment_Rij_vo_vo_PardoLoopManager();
	friend std::ostream& operator<<(std::ostream&,
				const Fragment_Rij_vo_vo_PardoLoopManager &);
private:
	virtual std::string to_string() const;
	virtual bool do_update();
	virtual void do_finalize();
	bool first_time_;
	int num_indices_;
	long& iteration_;
	index_selector_t index_id_;
	index_value_array_t lower_seg_;
	index_value_array_t upper_bound_;
	int num_where_clauses_;

	DataManager & data_manager_;
	const SipTables & sip_tables_;
	SIPMPIAttr & sip_mpi_attr_;
	int company_rank_;
	int num_workers_;
	Interpreter* interpreter_;

	int index_values_[MAX_RANK];

    bool initialize_indices();
	bool increment_simple_pair();
    bool increment_single_index(int index);
    bool increment_all();

    //double return_val_rcut_dist(int index1, int index2);
	//double return_val_elst_dist(int index1, int index2);
	//double return_val_swao_frag(int index1);
	//double return_val_swocca_frag(int index1);
	//double return_val_swvirta_frag(int index1);
    
    void form_rcut_dist();
    void form_elst_dist();
    void form_swao_frag();
    void form_swocca_frag();
    void form_swvirta_frag();
    
    bool fragment_special_where_clause(int typ, int index, int frag);
    bool where_clause(int index);
    
    std::vector< std::vector<int> > elst_dist;
    std::vector< std::vector<int> > rcut_dist;
    std::vector<int> swao_frag;
    std::vector<int> swocca_frag;
    std::vector<int> swvirta_frag;
    
};

class Fragment_i_aovo__PardoLoopManager: public LoopManager{
public:
	Fragment_i_aovo__PardoLoopManager(int num_indices,
			const int (&index_ids)[MAX_RANK], DataManager & data_manager,
			const SipTables & sip_tables, SIPMPIAttr& sip_mpi_attr,
			int num_where_clauses, Interpreter* interpreter, long& iteration);
	virtual ~Fragment_i_aovo__PardoLoopManager();
	friend std::ostream& operator<<(std::ostream&,
				const Fragment_i_aovo__PardoLoopManager &);
private:
	virtual std::string to_string() const;
	virtual bool do_update();
	virtual void do_finalize();
	bool first_time_;
	int num_indices_;
	long& iteration_;
	index_selector_t index_id_;
	index_value_array_t lower_seg_;
	index_value_array_t upper_bound_;
	int num_where_clauses_;

	DataManager & data_manager_;
	const SipTables & sip_tables_;
	SIPMPIAttr & sip_mpi_attr_;
	int company_rank_;
	int num_workers_;
	Interpreter* interpreter_;

	int index_values_[MAX_RANK];

    bool initialize_indices();
	bool increment_simple_pair();
    bool increment_single_index(int index);
    bool increment_all();

    //double return_val_rcut_dist(int index1, int index2);
	//double return_val_elst_dist(int index1, int index2);
	//double return_val_swao_frag(int index1);
	//double return_val_swocca_frag(int index1);
	//double return_val_swvirta_frag(int index1);
    
    void form_rcut_dist();
    void form_elst_dist();
    void form_swao_frag();
    void form_swocca_frag();
    void form_swvirta_frag();
    
    bool fragment_special_where_clause(int typ, int index, int frag);
    bool where_clause(int index);
    
    std::vector< std::vector<int> > elst_dist;
    std::vector< std::vector<int> > rcut_dist;
    std::vector<int> swao_frag;
    std::vector<int> swocca_frag;
    std::vector<int> swvirta_frag;
    
};

class Fragment_i_aaaa__PardoLoopManager: public LoopManager{
public:
	Fragment_i_aaaa__PardoLoopManager(int num_indices,
			const int (&index_ids)[MAX_RANK], DataManager & data_manager,
			const SipTables & sip_tables, SIPMPIAttr& sip_mpi_attr,
			int num_where_clauses, Interpreter* interpreter, long& iteration);
	virtual ~Fragment_i_aaaa__PardoLoopManager();
	friend std::ostream& operator<<(std::ostream&,
				const Fragment_i_aaaa__PardoLoopManager &);
private:
	virtual std::string to_string() const;
	virtual bool do_update();
	virtual void do_finalize();
	bool first_time_;
	int num_indices_;
	long& iteration_;
	index_selector_t index_id_;
	index_value_array_t lower_seg_;
	index_value_array_t upper_bound_;
	int num_where_clauses_;

	DataManager & data_manager_;
	const SipTables & sip_tables_;
	SIPMPIAttr & sip_mpi_attr_;
	int company_rank_;
	int num_workers_;
	Interpreter* interpreter_;

	int index_values_[MAX_RANK];

    bool initialize_indices();
	bool increment_simple_pair();
    bool increment_single_index(int index);
    bool increment_all();

    //double return_val_rcut_dist(int index1, int index2);
	//double return_val_elst_dist(int index1, int index2);
	//double return_val_swao_frag(int index1);
	//double return_val_swocca_frag(int index1);
	//double return_val_swvirta_frag(int index1);
    
    void form_rcut_dist();
    void form_elst_dist();
    void form_swao_frag();
    void form_swocca_frag();
    void form_swvirta_frag();
    
    bool fragment_special_where_clause(int typ, int index, int frag);
    bool where_clause(int index);
    
    std::vector< std::vector<int> > elst_dist;
    std::vector< std::vector<int> > rcut_dist;
    std::vector<int> swao_frag;
    std::vector<int> swocca_frag;
    std::vector<int> swvirta_frag;
    
};

class Fragment_i_aoo__PardoLoopManager: public LoopManager{
public:
	Fragment_i_aoo__PardoLoopManager(int num_indices,
			const int (&index_ids)[MAX_RANK], DataManager & data_manager,
			const SipTables & sip_tables, SIPMPIAttr& sip_mpi_attr,
			int num_where_clauses, Interpreter* interpreter, long& iteration);
	virtual ~Fragment_i_aoo__PardoLoopManager();
	friend std::ostream& operator<<(std::ostream&,
				const Fragment_i_aoo__PardoLoopManager &);
private:
	virtual std::string to_string() const;
	virtual bool do_update();
	virtual void do_finalize();
	bool first_time_;
	int num_indices_;
	long& iteration_;
	index_selector_t index_id_;
	index_value_array_t lower_seg_;
	index_value_array_t upper_bound_;
	int num_where_clauses_;

	DataManager & data_manager_;
	const SipTables & sip_tables_;
	SIPMPIAttr & sip_mpi_attr_;
	int company_rank_;
	int num_workers_;
	Interpreter* interpreter_;

	int index_values_[MAX_RANK];

    bool initialize_indices();
	bool increment_simple_pair();
    bool increment_single_index(int index);
    bool increment_all();

    //double return_val_rcut_dist(int index1, int index2);
	//double return_val_elst_dist(int index1, int index2);
	//double return_val_swao_frag(int index1);
	//double return_val_swocca_frag(int index1);
	//double return_val_swvirta_frag(int index1);
    
    void form_rcut_dist();
    void form_elst_dist();
    void form_swao_frag();
    void form_swocca_frag();
    void form_swvirta_frag();
    
    bool fragment_special_where_clause(int typ, int index, int frag);
    bool where_clause(int index);
    
    std::vector< std::vector<int> > elst_dist;
    std::vector< std::vector<int> > rcut_dist;
    std::vector<int> swao_frag;
    std::vector<int> swocca_frag;
    std::vector<int> swvirta_frag;
    
};

class Fragment_NRij_vovo__PardoLoopManager: public LoopManager{
public:
	Fragment_NRij_vovo__PardoLoopManager(int num_indices,
			const int (&index_ids)[MAX_RANK], DataManager & data_manager,
			const SipTables & sip_tables, SIPMPIAttr& sip_mpi_attr,
			int num_where_clauses, Interpreter* interpreter, long& iteration);
	virtual ~Fragment_NRij_vovo__PardoLoopManager();
	friend std::ostream& operator<<(std::ostream&,
				const Fragment_NRij_vovo__PardoLoopManager &);
private:
	virtual std::string to_string() const;
	virtual bool do_update();
	virtual void do_finalize();
	bool first_time_;
	int num_indices_;
	long& iteration_;
	index_selector_t index_id_;
	index_value_array_t lower_seg_;
	index_value_array_t upper_bound_;
	int num_where_clauses_;

	DataManager & data_manager_;
	const SipTables & sip_tables_;
	SIPMPIAttr & sip_mpi_attr_;
	int company_rank_;
	int num_workers_;
	Interpreter* interpreter_;

	int index_values_[MAX_RANK];

    bool initialize_indices();
	bool increment_simple_pair();
    bool increment_single_index(int index);
    bool increment_all();

    //double return_val_rcut_dist(int index1, int index2);
	//double return_val_elst_dist(int index1, int index2);
	//double return_val_swao_frag(int index1);
	//double return_val_swocca_frag(int index1);
	//double return_val_swvirta_frag(int index1);
    
    void form_rcut_dist();
    void form_elst_dist();
    void form_swao_frag();
    void form_swocca_frag();
    void form_swvirta_frag();
    
    bool fragment_special_where_clause(int typ, int index, int frag);
    bool where_clause(int index);
    
    std::vector< std::vector<int> > elst_dist;
    std::vector< std::vector<int> > rcut_dist;
    std::vector<int> swao_frag;
    std::vector<int> swocca_frag;
    std::vector<int> swvirta_frag;
    
};

class Fragment_NRij_ao_ao_PardoLoopManager: public LoopManager{
public:
	Fragment_NRij_ao_ao_PardoLoopManager(int num_indices,
			const int (&index_ids)[MAX_RANK], DataManager & data_manager,
			const SipTables & sip_tables, SIPMPIAttr& sip_mpi_attr,
			int num_where_clauses, Interpreter* interpreter, long& iteration);
	virtual ~Fragment_NRij_ao_ao_PardoLoopManager();
	friend std::ostream& operator<<(std::ostream&,
				const Fragment_NRij_ao_ao_PardoLoopManager &);
private:
	virtual std::string to_string() const;
	virtual bool do_update();
	virtual void do_finalize();
	bool first_time_;
	int num_indices_;
	long& iteration_;
	index_selector_t index_id_;
	index_value_array_t lower_seg_;
	index_value_array_t upper_bound_;
	int num_where_clauses_;

	DataManager & data_manager_;
	const SipTables & sip_tables_;
	SIPMPIAttr & sip_mpi_attr_;
	int company_rank_;
	int num_workers_;
	Interpreter* interpreter_;

	int index_values_[MAX_RANK];

    bool initialize_indices();
	bool increment_simple_pair();
    bool increment_single_index(int index);
    bool increment_all();

    //double return_val_rcut_dist(int index1, int index2);
	//double return_val_elst_dist(int index1, int index2);
	//double return_val_swao_frag(int index1);
	//double return_val_swocca_frag(int index1);
	//double return_val_swvirta_frag(int index1);
    
    void form_rcut_dist();
    void form_elst_dist();
    void form_swao_frag();
    void form_swocca_frag();
    void form_swvirta_frag();
    
    bool fragment_special_where_clause(int typ, int index, int frag);
    bool where_clause(int index);
    
    std::vector< std::vector<int> > elst_dist;
    std::vector< std::vector<int> > rcut_dist;
    std::vector<int> swao_frag;
    std::vector<int> swocca_frag;
    std::vector<int> swvirta_frag;
    
};

class Fragment_NRij_vo_ao_PardoLoopManager: public LoopManager{
public:
	Fragment_NRij_vo_ao_PardoLoopManager(int num_indices,
			const int (&index_ids)[MAX_RANK], DataManager & data_manager,
			const SipTables & sip_tables, SIPMPIAttr& sip_mpi_attr,
			int num_where_clauses, Interpreter* interpreter, long& iteration);
	virtual ~Fragment_NRij_vo_ao_PardoLoopManager();
	friend std::ostream& operator<<(std::ostream&,
				const Fragment_NRij_vo_ao_PardoLoopManager &);
private:
	virtual std::string to_string() const;
	virtual bool do_update();
	virtual void do_finalize();
	bool first_time_;
	int num_indices_;
	long& iteration_;
	index_selector_t index_id_;
	index_value_array_t lower_seg_;
	index_value_array_t upper_bound_;
	int num_where_clauses_;

	DataManager & data_manager_;
	const SipTables & sip_tables_;
	SIPMPIAttr & sip_mpi_attr_;
	int company_rank_;
	int num_workers_;
	Interpreter* interpreter_;

	int index_values_[MAX_RANK];

    bool initialize_indices();
	bool increment_simple_pair();
    bool increment_single_index(int index);
    bool increment_all();

    //double return_val_rcut_dist(int index1, int index2);
	//double return_val_elst_dist(int index1, int index2);
	//double return_val_swao_frag(int index1);
	//double return_val_swocca_frag(int index1);
	//double return_val_swvirta_frag(int index1);
    
    void form_rcut_dist();
    void form_elst_dist();
    void form_swao_frag();
    void form_swocca_frag();
    void form_swvirta_frag();
    
    bool fragment_special_where_clause(int typ, int index, int frag);
    bool where_clause(int index);
    
    std::vector< std::vector<int> > elst_dist;
    std::vector< std::vector<int> > rcut_dist;
    std::vector<int> swao_frag;
    std::vector<int> swocca_frag;
    std::vector<int> swvirta_frag;
    
};

class Fragment_NRij_aa_aa_PardoLoopManager: public LoopManager{
public:
	Fragment_NRij_aa_aa_PardoLoopManager(int num_indices,
			const int (&index_ids)[MAX_RANK], DataManager & data_manager,
			const SipTables & sip_tables, SIPMPIAttr& sip_mpi_attr,
			int num_where_clauses, Interpreter* interpreter, long& iteration);
	virtual ~Fragment_NRij_aa_aa_PardoLoopManager();
	friend std::ostream& operator<<(std::ostream&,
				const Fragment_NRij_aa_aa_PardoLoopManager &);
private:
	virtual std::string to_string() const;
	virtual bool do_update();
	virtual void do_finalize();
	bool first_time_;
	int num_indices_;
	long& iteration_;
	index_selector_t index_id_;
	index_value_array_t lower_seg_;
	index_value_array_t upper_bound_;
	int num_where_clauses_;

	DataManager & data_manager_;
	const SipTables & sip_tables_;
	SIPMPIAttr & sip_mpi_attr_;
	int company_rank_;
	int num_workers_;
	Interpreter* interpreter_;

	int index_values_[MAX_RANK];

    bool initialize_indices();
	bool increment_simple_pair();
    bool increment_single_index(int index);
    bool increment_all();

    //double return_val_rcut_dist(int index1, int index2);
	//double return_val_elst_dist(int index1, int index2);
	//double return_val_swao_frag(int index1);
	//double return_val_swocca_frag(int index1);
	//double return_val_swvirta_frag(int index1);
    
    void form_rcut_dist();
    void form_elst_dist();
    void form_swao_frag();
    void form_swocca_frag();
    void form_swvirta_frag();
    
    bool fragment_special_where_clause(int typ, int index, int frag);
    bool where_clause(int index);
    
    std::vector< std::vector<int> > elst_dist;
    std::vector< std::vector<int> > rcut_dist;
    std::vector<int> swao_frag;
    std::vector<int> swocca_frag;
    std::vector<int> swvirta_frag;
    
};

class Fragment_NRij_vv_oo_PardoLoopManager: public LoopManager{
public:
	Fragment_NRij_vv_oo_PardoLoopManager(int num_indices,
			const int (&index_ids)[MAX_RANK], DataManager & data_manager,
			const SipTables & sip_tables, SIPMPIAttr& sip_mpi_attr,
			int num_where_clauses, Interpreter* interpreter, long& iteration);
	virtual ~Fragment_NRij_vv_oo_PardoLoopManager();
	friend std::ostream& operator<<(std::ostream&,
				const Fragment_NRij_vv_oo_PardoLoopManager &);
private:
	virtual std::string to_string() const;
	virtual bool do_update();
	virtual void do_finalize();
	bool first_time_;
	int num_indices_;
	long& iteration_;
	index_selector_t index_id_;
	index_value_array_t lower_seg_;
	index_value_array_t upper_bound_;
	int num_where_clauses_;

	DataManager & data_manager_;
	const SipTables & sip_tables_;
	SIPMPIAttr & sip_mpi_attr_;
	int company_rank_;
	int num_workers_;
	Interpreter* interpreter_;

	int index_values_[MAX_RANK];

    bool initialize_indices();
	bool increment_simple_pair();
    bool increment_single_index(int index);
    bool increment_all();

    //double return_val_rcut_dist(int index1, int index2);
	//double return_val_elst_dist(int index1, int index2);
	//double return_val_swao_frag(int index1);
	//double return_val_swocca_frag(int index1);
	//double return_val_swvirta_frag(int index1);
    
    void form_rcut_dist();
    void form_elst_dist();
    void form_swao_frag();
    void form_swocca_frag();
    void form_swvirta_frag();
    
    bool fragment_special_where_clause(int typ, int index, int frag);
    bool where_clause(int index);
    
    std::vector< std::vector<int> > elst_dist;
    std::vector< std::vector<int> > rcut_dist;
    std::vector<int> swao_frag;
    std::vector<int> swocca_frag;
    std::vector<int> swvirta_frag;
    
};

class Fragment_NRij_o_ao_PardoLoopManager: public LoopManager{
public:
	Fragment_NRij_o_ao_PardoLoopManager(int num_indices,
			const int (&index_ids)[MAX_RANK], DataManager & data_manager,
			const SipTables & sip_tables, SIPMPIAttr& sip_mpi_attr,
			int num_where_clauses, Interpreter* interpreter, long& iteration);
	virtual ~Fragment_NRij_o_ao_PardoLoopManager();
	friend std::ostream& operator<<(std::ostream&,
				const Fragment_NRij_o_ao_PardoLoopManager &);
private:
	virtual std::string to_string() const;
	virtual bool do_update();
	virtual void do_finalize();
	bool first_time_;
	int num_indices_;
	long& iteration_;
	index_selector_t index_id_;
	index_value_array_t lower_seg_;
	index_value_array_t upper_bound_;
	int num_where_clauses_;

	DataManager & data_manager_;
	const SipTables & sip_tables_;
	SIPMPIAttr & sip_mpi_attr_;
	int company_rank_;
	int num_workers_;
	Interpreter* interpreter_;

	int index_values_[MAX_RANK];

    bool initialize_indices();
	bool increment_simple_pair();
    bool increment_single_index(int index);
    bool increment_all();

    //double return_val_rcut_dist(int index1, int index2);
	//double return_val_elst_dist(int index1, int index2);
	//double return_val_swao_frag(int index1);
	//double return_val_swocca_frag(int index1);
	//double return_val_swvirta_frag(int index1);
    
    void form_rcut_dist();
    void form_elst_dist();
    void form_swao_frag();
    void form_swocca_frag();
    void form_swvirta_frag();
    
    bool fragment_special_where_clause(int typ, int index, int frag);
    bool where_clause(int index);
    
    std::vector< std::vector<int> > elst_dist;
    std::vector< std::vector<int> > rcut_dist;
    std::vector<int> swao_frag;
    std::vector<int> swocca_frag;
    std::vector<int> swvirta_frag;
    
};

class Fragment_NR1ij_vo_vo_PardoLoopManager: public LoopManager{
public:
	Fragment_NR1ij_vo_vo_PardoLoopManager(int num_indices,
			const int (&index_ids)[MAX_RANK], DataManager & data_manager,
			const SipTables & sip_tables, SIPMPIAttr& sip_mpi_attr,
			int num_where_clauses, Interpreter* interpreter, long& iteration);
	virtual ~Fragment_NR1ij_vo_vo_PardoLoopManager();
	friend std::ostream& operator<<(std::ostream&,
				const Fragment_NR1ij_vo_vo_PardoLoopManager &);
private:
	virtual std::string to_string() const;
	virtual bool do_update();
	virtual void do_finalize();
	bool first_time_;
	int num_indices_;
	long& iteration_;
	index_selector_t index_id_;
	index_value_array_t lower_seg_;
	index_value_array_t upper_bound_;
	int num_where_clauses_;

	DataManager & data_manager_;
	const SipTables & sip_tables_;
	SIPMPIAttr & sip_mpi_attr_;
	int company_rank_;
	int num_workers_;
	Interpreter* interpreter_;

	int index_values_[MAX_RANK];

    bool initialize_indices();
	bool increment_simple_pair();
    bool increment_single_index(int index);
    bool increment_all();

    //double return_val_rcut_dist(int index1, int index2);
	//double return_val_elst_dist(int index1, int index2);
	//double return_val_swao_frag(int index1);
	//double return_val_swocca_frag(int index1);
	//double return_val_swvirta_frag(int index1);
    
    void form_rcut_dist();
    void form_elst_dist();
    void form_swao_frag();
    void form_swocca_frag();
    void form_swvirta_frag();
    
    bool fragment_special_where_clause(int typ, int index, int frag);
    bool where_clause(int index);
    
    std::vector< std::vector<int> > elst_dist;
    std::vector< std::vector<int> > rcut_dist;
    std::vector<int> swao_frag;
    std::vector<int> swocca_frag;
    std::vector<int> swvirta_frag;
    
};

class Fragment_NR1ij_oo_vo_PardoLoopManager: public LoopManager{
public:
	Fragment_NR1ij_oo_vo_PardoLoopManager(int num_indices,
			const int (&index_ids)[MAX_RANK], DataManager & data_manager,
			const SipTables & sip_tables, SIPMPIAttr& sip_mpi_attr,
			int num_where_clauses, Interpreter* interpreter, long& iteration);
	virtual ~Fragment_NR1ij_oo_vo_PardoLoopManager();
	friend std::ostream& operator<<(std::ostream&,
				const Fragment_NR1ij_oo_vo_PardoLoopManager &);
private:
	virtual std::string to_string() const;
	virtual bool do_update();
	virtual void do_finalize();
	bool first_time_;
	int num_indices_;
	long& iteration_;
	index_selector_t index_id_;
	index_value_array_t lower_seg_;
	index_value_array_t upper_bound_;
	int num_where_clauses_;

	DataManager & data_manager_;
	const SipTables & sip_tables_;
	SIPMPIAttr & sip_mpi_attr_;
	int company_rank_;
	int num_workers_;
	Interpreter* interpreter_;

	int index_values_[MAX_RANK];

    bool initialize_indices();
	bool increment_simple_pair();
    bool increment_single_index(int index);
    bool increment_all();

    //double return_val_rcut_dist(int index1, int index2);
	//double return_val_elst_dist(int index1, int index2);
	//double return_val_swao_frag(int index1);
	//double return_val_swocca_frag(int index1);
	//double return_val_swvirta_frag(int index1);
    
    void form_rcut_dist();
    void form_elst_dist();
    void form_swao_frag();
    void form_swocca_frag();
    void form_swvirta_frag();
    
    bool fragment_special_where_clause(int typ, int index, int frag);
    bool where_clause(int index);
    
    std::vector< std::vector<int> > elst_dist;
    std::vector< std::vector<int> > rcut_dist;
    std::vector<int> swao_frag;
    std::vector<int> swocca_frag;
    std::vector<int> swvirta_frag;
    
};

class Fragment_NR1ij_vv_vo_PardoLoopManager: public LoopManager{
public:
	Fragment_NR1ij_vv_vo_PardoLoopManager(int num_indices,
			const int (&index_ids)[MAX_RANK], DataManager & data_manager,
			const SipTables & sip_tables, SIPMPIAttr& sip_mpi_attr,
			int num_where_clauses, Interpreter* interpreter, long& iteration);
	virtual ~Fragment_NR1ij_vv_vo_PardoLoopManager();
	friend std::ostream& operator<<(std::ostream&,
				const Fragment_NR1ij_vv_vo_PardoLoopManager &);
private:
	virtual std::string to_string() const;
	virtual bool do_update();
	virtual void do_finalize();
	bool first_time_;
	int num_indices_;
	long& iteration_;
	index_selector_t index_id_;
	index_value_array_t lower_seg_;
	index_value_array_t upper_bound_;
	int num_where_clauses_;

	DataManager & data_manager_;
	const SipTables & sip_tables_;
	SIPMPIAttr & sip_mpi_attr_;
	int company_rank_;
	int num_workers_;
	Interpreter* interpreter_;

	int index_values_[MAX_RANK];

    bool initialize_indices();
	bool increment_simple_pair();
    bool increment_single_index(int index);
    bool increment_all();

    //double return_val_rcut_dist(int index1, int index2);
	//double return_val_elst_dist(int index1, int index2);
	//double return_val_swao_frag(int index1);
	//double return_val_swocca_frag(int index1);
	//double return_val_swvirta_frag(int index1);
    
    void form_rcut_dist();
    void form_elst_dist();
    void form_swao_frag();
    void form_swocca_frag();
    void form_swvirta_frag();
    
    bool fragment_special_where_clause(int typ, int index, int frag);
    bool where_clause(int index);
    
    std::vector< std::vector<int> > elst_dist;
    std::vector< std::vector<int> > rcut_dist;
    std::vector<int> swao_frag;
    std::vector<int> swocca_frag;
    std::vector<int> swvirta_frag;
    
};

#endif

} /* namespace sip */
#endif /* FRAGMENT_LOOP_MANAGER_H_ */
