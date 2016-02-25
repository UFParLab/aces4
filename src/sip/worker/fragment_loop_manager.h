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

class FragmentPardoLoopManager: public LoopManager {
protected:
	FragmentPardoLoopManager(int num_indices,
			const int (&index_id)[MAX_RANK],
			DataManager & data_manager,
			const SipTables & sip_tables);

	virtual std::string to_string() const;
	virtual void do_finalize();

	DataManager & data_manager_;
	const SipTables & sip_tables_;
	int index_values_[MAX_RANK];
	int num_indices_;
	index_selector_t index_id_;
	index_value_array_t lower_seg_;
	index_value_array_t upper_bound_;

	bool initialize_indices();
	bool increment_simple_pair();
	bool increment_single_index(int index);
	bool increment_all();

	void form_rcut_dist();
	void form_elst_dist();
	void form_swao_frag();
    void form_swmoa_frag();
	void form_swocca_frag();
	void form_swvirta_frag();

	bool fragment_special_where_clause(int typ, int index, int frag);

	std::vector<std::vector<int> > elst_dist;
	std::vector<std::vector<int> > rcut_dist;
	std::vector<int> swao_frag;
    std::vector<int> swmoa_frag;
	std::vector<int> swocca_frag;
	std::vector<int> swvirta_frag;
};

/**
 * Special Loop manager for Fragment code mcpt2_corr_lowmem.
 * Optimizes where clause lookups that involves looking up blocks.
 *
 *  naming scheme: _<simple index fragment indices>_<occ (o), virt (v), ao (a)
 *  in ifrag>_<o,v,a in jfrag>
 *
 *  */

class Fragment_Nij_aa__PardoLoopManager: public FragmentPardoLoopManager {
public:
	Fragment_Nij_aa__PardoLoopManager(int num_indices,
			const int (&index_ids)[MAX_RANK], DataManager & data_manager,
			const SipTables & sip_tables, SIPMPIAttr& sip_mpi_attr,
			int num_where_clauses, Interpreter* interpreter, long& iteration);
	virtual ~Fragment_Nij_aa__PardoLoopManager();
private:
	virtual bool do_update();
	bool where_clause(int index);

	bool first_time_;
	long& iteration_;
	int num_where_clauses_;

	SIPMPIAttr & sip_mpi_attr_;
	int company_rank_;
	int num_workers_;
	Interpreter* interpreter_;
};

class Fragment_Nij_a_a_PardoLoopManager: public FragmentPardoLoopManager {
public:
	Fragment_Nij_a_a_PardoLoopManager(int num_indices,
			const int (&index_ids)[MAX_RANK], DataManager & data_manager,
			const SipTables & sip_tables, SIPMPIAttr& sip_mpi_attr,
			int num_where_clauses, Interpreter* interpreter, long& iteration);
	virtual ~Fragment_Nij_a_a_PardoLoopManager();
private:
	virtual bool do_update();
	bool where_clause(int index);

	bool first_time_;
	long& iteration_;
	int num_where_clauses_;

	SIPMPIAttr & sip_mpi_attr_;
	int company_rank_;
	int num_workers_;
	Interpreter* interpreter_;
};

class Fragment_Nij_oo__PardoLoopManager: public FragmentPardoLoopManager {
public:
	Fragment_Nij_oo__PardoLoopManager(int num_indices,
			const int (&index_ids)[MAX_RANK], DataManager & data_manager,
			const SipTables & sip_tables, SIPMPIAttr& sip_mpi_attr,
			int num_where_clauses, Interpreter* interpreter, long& iteration);
	virtual ~Fragment_Nij_oo__PardoLoopManager();
private:
	virtual bool do_update();
	bool where_clause(int index);

	bool first_time_;
	long& iteration_;
	int num_where_clauses_;

	SIPMPIAttr & sip_mpi_attr_;
	int company_rank_;
	int num_workers_;
	Interpreter* interpreter_;
};

class Fragment_Nij_o_o_PardoLoopManager: public FragmentPardoLoopManager {
public:
	Fragment_Nij_o_o_PardoLoopManager(int num_indices,
			const int (&index_ids)[MAX_RANK], DataManager & data_manager,
			const SipTables & sip_tables, SIPMPIAttr& sip_mpi_attr,
			int num_where_clauses, Interpreter* interpreter, long& iteration);
	virtual ~Fragment_Nij_o_o_PardoLoopManager();
private:
	virtual bool do_update();
	bool where_clause(int index);

	bool first_time_;
	long& iteration_;
	int num_where_clauses_;

	SIPMPIAttr & sip_mpi_attr_;
	int company_rank_;
	int num_workers_;
	Interpreter* interpreter_;
};

class WhereFragment_i_aaa__PardoLoopManager: public FragmentPardoLoopManager {
public:
	WhereFragment_i_aaa__PardoLoopManager(int num_indices,
			const int (&index_ids)[MAX_RANK], DataManager & data_manager,
			const SipTables & sip_tables, SIPMPIAttr& sip_mpi_attr,
			int num_where_clauses, Interpreter* interpreter, long& iteration);
	virtual ~WhereFragment_i_aaa__PardoLoopManager();
private:
	virtual bool do_update();
	bool where_clause(int index);

	bool first_time_;
	long& iteration_;
	int num_where_clauses_;

	SIPMPIAttr & sip_mpi_attr_;
	int company_rank_;
	int num_workers_;
	Interpreter* interpreter_;
};

class WhereFragment_i_aaaa__PardoLoopManager: public FragmentPardoLoopManager {
public:
	WhereFragment_i_aaaa__PardoLoopManager(int num_indices,
			const int (&index_ids)[MAX_RANK], DataManager & data_manager,
			const SipTables & sip_tables, SIPMPIAttr& sip_mpi_attr,
			int num_where_clauses, Interpreter* interpreter, long& iteration);
	virtual ~WhereFragment_i_aaaa__PardoLoopManager();
private:
	virtual bool do_update();
	bool where_clause(int index);

	bool first_time_;
	long& iteration_;
	int num_where_clauses_;

	SIPMPIAttr & sip_mpi_attr_;
	int company_rank_;
	int num_workers_;
	Interpreter* interpreter_;
};

class Fragment_i_aa__PardoLoopManager: public FragmentPardoLoopManager {
public:
	Fragment_i_aa__PardoLoopManager(int num_indices,
			const int (&index_ids)[MAX_RANK], DataManager & data_manager,
			const SipTables & sip_tables, SIPMPIAttr& sip_mpi_attr,
			int num_where_clauses, Interpreter* interpreter, long& iteration);
	virtual ~Fragment_i_aa__PardoLoopManager();
private:
	virtual bool do_update();
	bool where_clause(int index);

	bool first_time_;
	long& iteration_;
	int num_where_clauses_;

	SIPMPIAttr & sip_mpi_attr_;
	int company_rank_;
	int num_workers_;
	Interpreter* interpreter_;
};

class Fragment_ij_aaa__PardoLoopManager: public FragmentPardoLoopManager {
public:
	Fragment_ij_aaa__PardoLoopManager(int num_indices,
			const int (&index_ids)[MAX_RANK], DataManager & data_manager,
			const SipTables & sip_tables, SIPMPIAttr& sip_mpi_attr,
			int num_where_clauses, Interpreter* interpreter, long& iteration);
	virtual ~Fragment_ij_aaa__PardoLoopManager();
private:
	virtual bool do_update();
	bool where_clause(int index);

	bool first_time_;
	long& iteration_;
	int num_where_clauses_;

	SIPMPIAttr & sip_mpi_attr_;
	int company_rank_;
	int num_workers_;
	Interpreter* interpreter_;
};

class Fragment_ij_aa_a_PardoLoopManager: public FragmentPardoLoopManager {
public:
	Fragment_ij_aa_a_PardoLoopManager(int num_indices,
			const int (&index_ids)[MAX_RANK], DataManager & data_manager,
			const SipTables & sip_tables, SIPMPIAttr& sip_mpi_attr,
			int num_where_clauses, Interpreter* interpreter, long& iteration);
	virtual ~Fragment_ij_aa_a_PardoLoopManager();
private:
	virtual bool do_update();
	bool where_clause(int index);

	bool first_time_;
	long& iteration_;
	int num_where_clauses_;

	SIPMPIAttr & sip_mpi_attr_;
	int company_rank_;
	int num_workers_;
	Interpreter* interpreter_;
};

class Fragment_ij_ao_ao_PardoLoopManager: public FragmentPardoLoopManager {
public:
	Fragment_ij_ao_ao_PardoLoopManager(int num_indices,
			const int (&index_ids)[MAX_RANK], DataManager & data_manager,
			const SipTables & sip_tables, SIPMPIAttr& sip_mpi_attr,
			int num_where_clauses, Interpreter* interpreter, long& iteration);
	virtual ~Fragment_ij_ao_ao_PardoLoopManager();
private:
	virtual bool do_update();
	bool where_clause(int index);

	bool first_time_;
	long& iteration_;
	int num_where_clauses_;

	SIPMPIAttr & sip_mpi_attr_;
	int company_rank_;
	int num_workers_;
	Interpreter* interpreter_;
};

class Fragment_ij_aa_vo_PardoLoopManager: public FragmentPardoLoopManager {
public:
	Fragment_ij_aa_vo_PardoLoopManager(int num_indices,
			const int (&index_ids)[MAX_RANK], DataManager & data_manager,
			const SipTables & sip_tables, SIPMPIAttr& sip_mpi_attr,
			int num_where_clauses, Interpreter* interpreter, long& iteration);
	virtual ~Fragment_ij_aa_vo_PardoLoopManager();
private:
	virtual bool do_update();
	bool where_clause(int index);

	bool first_time_;
	long& iteration_;
	int num_where_clauses_;

	SIPMPIAttr & sip_mpi_attr_;
	int company_rank_;
	int num_workers_;
	Interpreter* interpreter_;
};

class Fragment_ij_aa_oo_PardoLoopManager: public FragmentPardoLoopManager {
public:
	Fragment_ij_aa_oo_PardoLoopManager(int num_indices,
			const int (&index_ids)[MAX_RANK], DataManager & data_manager,
			const SipTables & sip_tables, SIPMPIAttr& sip_mpi_attr,
			int num_where_clauses, Interpreter* interpreter, long& iteration);
	virtual ~Fragment_ij_aa_oo_PardoLoopManager();
private:
	virtual bool do_update();
	bool where_clause(int index);

	bool first_time_;
	long& iteration_;
	int num_where_clauses_;

	SIPMPIAttr & sip_mpi_attr_;
	int company_rank_;
	int num_workers_;
	Interpreter* interpreter_;
};

class Fragment_i_aaoo__PardoLoopManager: public FragmentPardoLoopManager {
public:
	Fragment_i_aaoo__PardoLoopManager(int num_indices,
			const int (&index_ids)[MAX_RANK], DataManager & data_manager,
			const SipTables & sip_tables, SIPMPIAttr& sip_mpi_attr,
			int num_where_clauses, Interpreter* interpreter, long& iteration);
	virtual ~Fragment_i_aaoo__PardoLoopManager();
private:
	virtual bool do_update();
	bool where_clause(int index);

	bool first_time_;
	long& iteration_;
	int num_where_clauses_;

	SIPMPIAttr & sip_mpi_attr_;
	int company_rank_;
	int num_workers_;
	Interpreter* interpreter_;
};

class Fragment_ij_aoa_o_PardoLoopManager: public FragmentPardoLoopManager {
public:
	Fragment_ij_aoa_o_PardoLoopManager(int num_indices,
			const int (&index_ids)[MAX_RANK], DataManager & data_manager,
			const SipTables & sip_tables, SIPMPIAttr& sip_mpi_attr,
			int num_where_clauses, Interpreter* interpreter, long& iteration);
	virtual ~Fragment_ij_aoa_o_PardoLoopManager();
private:
	virtual bool do_update();
	bool where_clause(int index);

	bool first_time_;
	long& iteration_;
	int num_where_clauses_;

	SIPMPIAttr & sip_mpi_attr_;
	int company_rank_;
	int num_workers_;
	Interpreter* interpreter_;
};

class Fragment_ij_aoo_o_PardoLoopManager: public FragmentPardoLoopManager {
public:
	Fragment_ij_aoo_o_PardoLoopManager(int num_indices,
			const int (&index_ids)[MAX_RANK], DataManager & data_manager,
			const SipTables & sip_tables, SIPMPIAttr& sip_mpi_attr,
			int num_where_clauses, Interpreter* interpreter, long& iteration);
	virtual ~Fragment_ij_aoo_o_PardoLoopManager();
private:
	virtual bool do_update();
	bool where_clause(int index);

	bool first_time_;
	long& iteration_;
	int num_where_clauses_;

	SIPMPIAttr & sip_mpi_attr_;
	int company_rank_;
	int num_workers_;
	Interpreter* interpreter_;
};

class Fragment_i_aovo__PardoLoopManager: public FragmentPardoLoopManager {
public:
	Fragment_i_aovo__PardoLoopManager(int num_indices,
			const int (&index_ids)[MAX_RANK], DataManager & data_manager,
			const SipTables & sip_tables, SIPMPIAttr& sip_mpi_attr,
			int num_where_clauses, Interpreter* interpreter, long& iteration);
	virtual ~Fragment_i_aovo__PardoLoopManager();
private:
	virtual bool do_update();
	bool where_clause(int index);

	bool first_time_;
	long& iteration_;
	int num_where_clauses_;

	SIPMPIAttr & sip_mpi_attr_;
	int company_rank_;
	int num_workers_;
	Interpreter* interpreter_;
};

class Fragment_i_aaaa__PardoLoopManager: public FragmentPardoLoopManager {
public:
	Fragment_i_aaaa__PardoLoopManager(int num_indices,
			const int (&index_ids)[MAX_RANK], DataManager & data_manager,
			const SipTables & sip_tables, SIPMPIAttr& sip_mpi_attr,
			int num_where_clauses, Interpreter* interpreter, long& iteration);
	virtual ~Fragment_i_aaaa__PardoLoopManager();
private:
	virtual bool do_update();
	bool where_clause(int index);

	bool first_time_;
	long& iteration_;
	int num_where_clauses_;

	SIPMPIAttr & sip_mpi_attr_;
	int company_rank_;
	int num_workers_;
	Interpreter* interpreter_;
};

class Fragment_i_aoo__PardoLoopManager: public FragmentPardoLoopManager {
public:
	Fragment_i_aoo__PardoLoopManager(int num_indices,
			const int (&index_ids)[MAX_RANK], DataManager & data_manager,
			const SipTables & sip_tables, SIPMPIAttr& sip_mpi_attr,
			int num_where_clauses, Interpreter* interpreter, long& iteration);
	virtual ~Fragment_i_aoo__PardoLoopManager();
private:
	virtual bool do_update();
	bool where_clause(int index);

	bool first_time_;
	long& iteration_;
	int num_where_clauses_;

	SIPMPIAttr & sip_mpi_attr_;
	int company_rank_;
	int num_workers_;
	Interpreter* interpreter_;
};

class Fragment_NRij_vovo__PardoLoopManager: public FragmentPardoLoopManager {
public:
	Fragment_NRij_vovo__PardoLoopManager(int num_indices,
			const int (&index_ids)[MAX_RANK], DataManager & data_manager,
			const SipTables & sip_tables, SIPMPIAttr& sip_mpi_attr,
			int num_where_clauses, Interpreter* interpreter, long& iteration);
	virtual ~Fragment_NRij_vovo__PardoLoopManager();
private:
	virtual bool do_update();
	bool where_clause(int index);

	bool first_time_;
	long& iteration_;
	int num_where_clauses_;

	SIPMPIAttr & sip_mpi_attr_;
	int company_rank_;
	int num_workers_;
	Interpreter* interpreter_;
};

class Fragment_NRij_ao_ao_PardoLoopManager: public FragmentPardoLoopManager {
public:
	Fragment_NRij_ao_ao_PardoLoopManager(int num_indices,
			const int (&index_ids)[MAX_RANK], DataManager & data_manager,
			const SipTables & sip_tables, SIPMPIAttr& sip_mpi_attr,
			int num_where_clauses, Interpreter* interpreter, long& iteration);
	virtual ~Fragment_NRij_ao_ao_PardoLoopManager();
private:
	virtual bool do_update();
	bool where_clause(int index);

	bool first_time_;
	long& iteration_;
	int num_where_clauses_;

	SIPMPIAttr & sip_mpi_attr_;
	int company_rank_;
	int num_workers_;
	Interpreter* interpreter_;
};

class Fragment_NRij_vo_ao_PardoLoopManager: public FragmentPardoLoopManager {
public:
	Fragment_NRij_vo_ao_PardoLoopManager(int num_indices,
			const int (&index_ids)[MAX_RANK], DataManager & data_manager,
			const SipTables & sip_tables, SIPMPIAttr& sip_mpi_attr,
			int num_where_clauses, Interpreter* interpreter, long& iteration);
	virtual ~Fragment_NRij_vo_ao_PardoLoopManager();
private:
	virtual bool do_update();
	bool where_clause(int index);

	bool first_time_;
	long& iteration_;
	int num_where_clauses_;

	SIPMPIAttr & sip_mpi_attr_;
	int company_rank_;
	int num_workers_;
	Interpreter* interpreter_;
};

class Fragment_NRij_aa_aa_PardoLoopManager: public FragmentPardoLoopManager {
public:
	Fragment_NRij_aa_aa_PardoLoopManager(int num_indices,
			const int (&index_ids)[MAX_RANK], DataManager & data_manager,
			const SipTables & sip_tables, SIPMPIAttr& sip_mpi_attr,
			int num_where_clauses, Interpreter* interpreter, long& iteration);
	virtual ~Fragment_NRij_aa_aa_PardoLoopManager();
private:
	virtual bool do_update();
	bool where_clause(int index);

	bool first_time_;
	long& iteration_;
	int num_where_clauses_;

	SIPMPIAttr & sip_mpi_attr_;
	int company_rank_;
	int num_workers_;
	Interpreter* interpreter_;
};

class Fragment_NRij_o_ao_PardoLoopManager: public FragmentPardoLoopManager {
public:
	Fragment_NRij_o_ao_PardoLoopManager(int num_indices,
			const int (&index_ids)[MAX_RANK], DataManager & data_manager,
			const SipTables & sip_tables, SIPMPIAttr& sip_mpi_attr,
			int num_where_clauses, Interpreter* interpreter, long& iteration);
	virtual ~Fragment_NRij_o_ao_PardoLoopManager();
private:
	virtual bool do_update();
	bool where_clause(int index);

	bool first_time_;
	long& iteration_;
	int num_where_clauses_;

	SIPMPIAttr & sip_mpi_attr_;
	int company_rank_;
	int num_workers_;
	Interpreter* interpreter_;
};

    class Fragment_i_ap__PardoLoopManager: public FragmentPardoLoopManager {
    public:
        Fragment_i_ap__PardoLoopManager(int num_indices,
                                        const int (&index_ids)[MAX_RANK], DataManager & data_manager,
                                        const SipTables & sip_tables, SIPMPIAttr& sip_mpi_attr,
                                        int num_where_clauses, Interpreter* interpreter, long& iteration);
        virtual ~Fragment_i_ap__PardoLoopManager();
    private:
        virtual bool do_update();
        bool where_clause(int index);
        
        bool first_time_;
        long& iteration_;
        int num_where_clauses_;
        
        SIPMPIAttr & sip_mpi_attr_;
        int company_rank_;
        int num_workers_;
        Interpreter* interpreter_;
    };

    class Fragment_i_pp__PardoLoopManager: public FragmentPardoLoopManager {
    public:
        Fragment_i_pp__PardoLoopManager(int num_indices,
                                        const int (&index_ids)[MAX_RANK], DataManager & data_manager,
                                        const SipTables & sip_tables, SIPMPIAttr& sip_mpi_attr,
                                        int num_where_clauses, Interpreter* interpreter, long& iteration);
        virtual ~Fragment_i_pp__PardoLoopManager();
    private:
        virtual bool do_update();
        bool where_clause(int index);
        
        bool first_time_;
        long& iteration_;
        int num_where_clauses_;
        
        SIPMPIAttr & sip_mpi_attr_;
        int company_rank_;
        int num_workers_;
        Interpreter* interpreter_;
    };
    
class Fragment_i_pppp__PardoLoopManager: public FragmentPardoLoopManager {
public:
    Fragment_i_pppp__PardoLoopManager(int num_indices,
                                          const int (&index_ids)[MAX_RANK], DataManager & data_manager,
                                          const SipTables & sip_tables, SIPMPIAttr& sip_mpi_attr,
                                          int num_where_clauses, Interpreter* interpreter, long& iteration);
        virtual ~Fragment_i_pppp__PardoLoopManager();
private:
    virtual bool do_update();
    bool where_clause(int index);
        
    bool first_time_;
    long& iteration_;
    int num_where_clauses_;
        
    SIPMPIAttr & sip_mpi_attr_;
    int company_rank_;
    int num_workers_;
    Interpreter* interpreter_;
};
    
    class Fragment_ij_ap_pp_PardoLoopManager: public FragmentPardoLoopManager {
    public:
        Fragment_ij_ap_pp_PardoLoopManager(int num_indices,
                                           const int (&index_ids)[MAX_RANK], DataManager & data_manager,
                                           const SipTables & sip_tables, SIPMPIAttr& sip_mpi_attr,
                                           int num_where_clauses, Interpreter* interpreter, long& iteration);
        virtual ~Fragment_ij_ap_pp_PardoLoopManager();
    private:
        virtual bool do_update();
        bool where_clause(int index);
        
        bool first_time_;
        long& iteration_;
        int num_where_clauses_;
        
        SIPMPIAttr & sip_mpi_attr_;
        int company_rank_;
        int num_workers_;
        Interpreter* interpreter_;
    };
    
    class Fragment_ij_pp_pp_PardoLoopManager: public FragmentPardoLoopManager {
    public:
        Fragment_ij_pp_pp_PardoLoopManager(int num_indices,
                                           const int (&index_ids)[MAX_RANK], DataManager & data_manager,
                                           const SipTables & sip_tables, SIPMPIAttr& sip_mpi_attr,
                                           int num_where_clauses, Interpreter* interpreter, long& iteration);
        virtual ~Fragment_ij_pp_pp_PardoLoopManager();
    private:
        virtual bool do_update();
        bool where_clause(int index);
        
        bool first_time_;
        long& iteration_;
        int num_where_clauses_;
        
        SIPMPIAttr & sip_mpi_attr_;
        int company_rank_;
        int num_workers_;
        Interpreter* interpreter_;
    };
    
    class Fragment_Nij_pp_pp_PardoLoopManager: public FragmentPardoLoopManager {
    public:
        Fragment_Nij_pp_pp_PardoLoopManager(int num_indices,
                                            const int (&index_ids)[MAX_RANK], DataManager & data_manager,
                                            const SipTables & sip_tables, SIPMPIAttr& sip_mpi_attr,
                                            int num_where_clauses, Interpreter* interpreter, long& iteration);
        virtual ~Fragment_Nij_pp_pp_PardoLoopManager();
    private:
        virtual bool do_update();
        bool where_clause(int index);
        
        bool first_time_;
        long& iteration_;
        int num_where_clauses_;
        
        SIPMPIAttr & sip_mpi_attr_;
        int company_rank_;
        int num_workers_;
        Interpreter* interpreter_;
    };
    
    class Fragment_Rij_pp_pp_PardoLoopManager: public FragmentPardoLoopManager {
    public:
        Fragment_Rij_pp_pp_PardoLoopManager(int num_indices,
                                            const int (&index_ids)[MAX_RANK], DataManager & data_manager,
                                            const SipTables & sip_tables, SIPMPIAttr& sip_mpi_attr,
                                            int num_where_clauses, Interpreter* interpreter, long& iteration);
        virtual ~Fragment_Rij_pp_pp_PardoLoopManager();
    private:
        virtual bool do_update();
        bool where_clause(int index);
        
        bool first_time_;
        long& iteration_;
        int num_where_clauses_;
        
        SIPMPIAttr & sip_mpi_attr_;
        int company_rank_;
        int num_workers_;
        Interpreter* interpreter_;
    };
    
    class Fragment_NRij_pp_pp_PardoLoopManager: public FragmentPardoLoopManager {
    public:
        Fragment_NRij_pp_pp_PardoLoopManager(int num_indices,
                                        const int (&index_ids)[MAX_RANK], DataManager & data_manager,
                                        const SipTables & sip_tables, SIPMPIAttr& sip_mpi_attr,
                                        int num_where_clauses, Interpreter* interpreter, long& iteration);
        virtual ~Fragment_NRij_pp_pp_PardoLoopManager();
    private:
        virtual bool do_update();
        bool where_clause(int index);
        
        bool first_time_;
        long& iteration_;
        int num_where_clauses_;
        
        SIPMPIAttr & sip_mpi_attr_;
        int company_rank_;
        int num_workers_;
        Interpreter* interpreter_;
    };
    
#endif

} /* namespace sip */
#endif /* FRAGMENT_LOOP_MANAGER_H_ */
