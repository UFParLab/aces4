/*
 * blocks.cpp
 *
 *  Created on: Aug 8, 2013
 *      Author: basbas
 */

#include "config.h"
#include "blocks.h"
#include <iostream>
#include <cstring>
#include "sip.h"
#include "tensor_ops_c_prototypes.h"
#include "sip_tables.h"

#ifdef HAVE_MPI
#include "sip_mpi_utils.h"
#endif //HAVE_MPI
#include "gpu_super_instructions.h"



namespace sip {


/********************** Block Shape ***********************/

BlockShape::BlockShape() {
	std::fill(segment_sizes_ + 0, segment_sizes_ + MAX_RANK, 1);
}

BlockShape::BlockShape(const segment_size_array_t& segment_sizes) {
	std::copy(segment_sizes + 0, segment_sizes + MAX_RANK, segment_sizes_ + 0);
}

BlockShape::~BlockShape() {
}std::ostream& operator<<(std::ostream& os, const BlockShape & shape) {
	os << "[";
	for (int i = 0; i < MAX_RANK; ++i) {
		os << (i == 0 ? "" : ",") << shape.segment_sizes_[i];
	}
	os << ']';

	return os;
}

int BlockShape::num_elems() const{
	int num_elems = 1;
	for (int i = 0; i < MAX_RANK; i++) {
		num_elems *= segment_sizes_[i];
	}
	return num_elems;
}

bool BlockShape::operator==(const BlockShape& rhs) const {
	bool is_equal = true;
	for (int i = 0; is_equal && i < MAX_RANK; ++i) {
		is_equal = (segment_sizes_[i] == rhs.segment_sizes_[i]);
	}
	return is_equal;
}
bool BlockShape::operator<(const BlockShape& rhs) const {
	bool is_eq = true;
	bool is_leq = true;
	for (int i = 0; is_leq && i < MAX_RANK; ++i) {
		is_leq = (segment_sizes_[i] <= rhs.segment_sizes_[i]);
		is_eq = is_eq && (segment_sizes_[i] == rhs.segment_sizes_[i]);
	}
	return (is_leq && !is_eq);
}

/*********************** Block Selector *****************/
//const int BlockSelector::unused_index = -2;
//const int BlockSelector::wild_card_index = 90909;
BlockSelector::BlockSelector(int array_id, int rank,
		const index_selector_t& index_ids) :
		array_id_(array_id),
		rank_(rank){
	for (int i = 0; i != rank; ++i) {
		index_ids_[i] = index_ids[i];
	}
	for (int i = rank; i != MAX_RANK; ++i) {
		index_ids_[i] = unused_index_slot;
	}
}

bool BlockSelector::operator==(const BlockSelector& rhs) const {
	if (rank_ != rhs.rank_) return false;
	for (int i = 0; i < MAX_RANK; ++i) {
		if  (index_ids_[i] != rhs.index_ids_[i]) return false;
	}
	return true;
}

std::ostream& operator<<(std::ostream& os, const BlockSelector & selector) {
	os << selector.array_id_;
	os << "[" << selector.index_ids_[0];
	for (int i = 1; i < MAX_RANK; ++i) {
		int id = selector.index_ids_[i];
		if (id != unused_index_slot)
			os << "," << id;
	}
	os << ']';
	return os;
}

/*********************** Block ***************************/

Block::Block(BlockShape shape) :
		shape_(shape)
#ifdef HAVE_MPI
	,mpi_request_(MPI_REQUEST_NULL)
#endif //HAVE_MPI
{
	size_ = shape.num_elems();
	data_ = new double[size_](); //c++ feature:parens cause allocated memory to be initialized to zero

	gpu_data_ = NULL;
	status_[Block::onHost] = true;
	status_[Block::onGPU] = false;
	status_[Block::dirtyOnHost] = false;
	status_[Block::dirtyOnGPU] = false;

}

//TODO consider whether data should be a Vector, or otherwise add checks for
//arrays out of bounds
Block::Block(BlockShape shape, dataPtr data):
		shape_(shape),
		data_(data)
#ifdef HAVE_MPI
	,mpi_request_(MPI_REQUEST_NULL)
#endif //HAVE_MPI
{
	size_ = shape.num_elems();

	gpu_data_ = NULL;
	status_[Block::onHost] = true;
	status_[Block::onGPU] = false;
	status_[Block::dirtyOnHost] = false;
	status_[Block::dirtyOnGPU] = false;
}

Block::Block(dataPtr data):
	data_(data)
#ifdef HAVE_MPI
	,mpi_request_(MPI_REQUEST_NULL)
#endif //HAVE_MPI
{
	std::fill(shape_.segment_sizes_+0, shape_.segment_sizes_+MAX_RANK, 1);
	size_ = 1;

	gpu_data_ = NULL;
	status_[Block::onHost] = true;
	status_[Block::onGPU] = false;
	status_[Block::dirtyOnHost] = false;
	status_[Block::dirtyOnGPU] = false;
}

//Block::BlockPtr Block::clone(){
//	//not fixed for MPI
//	//should not be used
//	fail("Block::clone not implemeted");
//	BlockPtr bptr = new Block();
//	bptr->data_ = new double[this->size_];
//	std::copy(data_, data_ + size_, bptr->data_);
//	bptr->size_ = this->size_;
//	bptr->shape_ = this->shape_;
//	bptr->status_ = this->status_;
//#ifdef HAVE_CUDA
//	if (this->gpu_data_ != NULL)
//		_gpu_device_to_device(bptr->gpu_data_, this->gpu_data_, size_);
//#endif
//	return bptr;
//}


Block::Block()
#ifdef HAVE_MPI
	:mpi_request_(MPI_REQUEST_NULL)
#endif //HAVE_MPI
{
	data_ = NULL;
	gpu_data_ = NULL;
	size_ = 0;
}

Block::~Block() {
	sip::check_and_warn((data_), std::string("in ~Block with NULL data_"));

#ifdef HAVE_MPI
	//check to see if block is in transit.  If this is the case, there was a get
	//on a block that was never used.  Print a warning.  We probably want to be able
	//disable this check.
	int flag;
	SIPMPIUtils::check_err(MPI_Test(&mpi_request_, &flag, MPI_STATUS_IGNORE));
	if (check_and_warn(flag, "deleting block with pending request. This likely is due to an unused get in the sial program")){
		SIPMPIUtils::check_err(MPI_Wait(&mpi_request_, MPI_STATUS_IGNORE));
	}
#endif //HAVE_MPI
	if (data_ != NULL && size_ >1) { //Assumption: if size==1, data_ points into the scalar table.
		delete[] data_;
		data_ = NULL;
	}
#ifdef HAVE_CUDA
	if (gpu_data_){
		std::cout<<"Now freeing gpu_data"<<std::endl;
		_gpu_free(gpu_data_);
		gpu_data_ = NULL;
	}
#endif //HAVE_CUDA
	status_[Block::onGPU] = false;
	status_[Block::onHost] = false;
	status_[Block::dirtyOnHost] = false;
	status_[Block::dirtyOnGPU] = false;
}

int Block::size() {
	return size_;
}

BlockShape Block::shape() {
	return shape_;
}

//Block::dataPtr Block::copy_data(BlockPtr source_block) {
//	return copy_data_(source_block, 0);
//}

Block::dataPtr Block::copy_data_(BlockPtr source_block, int offset) {
	dataPtr target = get_data();
	dataPtr source = source_block->get_data();
	int n = size(); //copy the number of elements needed by this block, which may be
					//less than in source_block.  This requires, and should be checked
					//by compiler that for, say, a[i,j] = a[i,j,k] that k is a simple index.
					//the caller needs to calculate and pass in the offset.
	sip::check(target != NULL, "Cannot copy data, target is NULL", current_line());
	sip::check(source != NULL, "Cannot copy data, source is NULL", current_line());
	std::copy(source+ offset, source + offset + n, target);
	return target;
}



Block::dataPtr Block::get_data() {
	return data_;
}


//TODO compare with std::fill
Block::dataPtr Block::fill(double value) {
	int ierr = 0;
//	dataPtr data = get_data();
	int rank = MAX_RANK;
	/*void tensor_block_init__(int&, double*, int&, int*, double&, int&);
	    subroutine tensor_block_init_(nthreads,tens,tens_rank,tens_dim_extents,sval,ierr) bind(c,name='tensor_block_init__') !PARALLEL
	 !This subroutine initializes a tensor block <tens> with some scalar value <sval>.
	 !INPUT:
	 ! - nthreads - number of threads requested;
	 ! - tens - tensor block;
	 ! - tens_rank - tensor rank;
	 ! - tens_dim_extents(1:tens_rank) - tensor dimension extents;
	 ! - sval - initialization scalar value <val>;
	 !OUTPUT:
	 ! - tens - filled tensor block;
	 ! - ierr - error code (0: success):
	 */
//	int nthreads = sip::MAX_OMP_THREADS;
//	tensor_block_init__(nthreads, data_, rank, shape_.segment_sizes_, value,
//			ierr);
//	sip::check(ierr == 0, "error returned from tensor_block_init_");
	std::fill(data_+0, data_+size(), value);
	return data_;
}

// TODO use Dmitry's
Block::dataPtr Block::scale(double value) {
	dataPtr ptr = get_data();
	int n = size();
	for (int i = 0; i < n; ++i) {
		ptr[i] *= value;
	}
	return ptr;
}

//void tensor_block_copy_(int&,int*,int*,double*,double*,int&);
Block::dataPtr Block::transpose_copy(BlockPtr source, int rank,
		permute_t& permute) {
	int ierr = 0;
	dataPtr data = get_data();
	dataPtr source_data = source->get_data();
	//Dmitry's permute routine expects the first element of the permute vector to be 1 (in general, 1 or -1, in aces, always 1)
	//followed by the permutation in terms of fortran indices.  So for example, the permutation index for
	//   a[i,j] = b[j,i] would be [1,2,1,...]
	//  To accomplish this, copy the elements of the permute vector to a new array.
	//  This is not a particularly frequent operation, so we will keep this here together with the call to the permute routine rather
	//  than moving it up in the interpreter to avoid the copy.
	int dmitry_permute[MAX_RANK + 1];
	dmitry_permute[0] = 1;
	for (int i = 0; i < MAX_RANK; ++i) {
		dmitry_permute[i + 1] = permute[i] + 1;
	} //modify permutation vector for fortran

	/*subroutine tensor_block_copy_(nthreads,dim_num,dim_extents,dim_transp,tens_in,tens_out,ierr) bind(c,name='tensor_block_copy__') !PARALLEL
	 !Given a dense tensor block, this subroutine makes a copy of it, permuting the indices according to the <dim_transp>.
	 !The algorithm is expected to be cache-efficient (Author: Dmitry I. Lyakh: quant4me@gmail.com).
	 !In practice, regardless of the index permutation, it should be only two times slower than the direct copy.
	 !INPUT:
	 ! - nthreads - number of threads requested;
	 ! - dim_num - number of dimensions (>0);
	 ! - dim_extents(1:dim_num) - dimension extents;
	 ! - dim_transp(0:dim_num) - index permutation (old-to-new);
	 ! - tens_in(0:) - input tensor block;
	 !OUTPUT:
	 ! - tens_out(0:) - output (possibly transposed) tensor block;
	 ! - ierr - error code (0:success).
	 !NOTES:
	 ! - The output tensor block is expected to have the same size as the input tensor block.
	 */
//std::cout<<"permutation ";
//for (int i = 0; i < MAX_RANK + 1; ++i) {
//	std::cout<<dmitry_permute[i] << " ";
//}
//std::cout<<std::endl;
	int nthreads = sip::MAX_OMP_THREADS;
	tensor_block_copy__(nthreads, rank, source->shape_.segment_sizes_,
			dmitry_permute, source_data, data, ierr);
	sip::check(ierr == 0,
			"error returned from tensor_block_copy_ on transpose");
	return data;
}


//TODO use Dimitry's routine
Block::dataPtr Block::accumulate_data(BlockPtr source) {
	sip::check(this->shape_ == source->shape_,
			" += applied to blocks with different shapes");
	dataPtr source_data =  source->data_;
	int n = size();
	for (int i = 0; i < n; ++i) {
		data_[i] += source_data[i];
	}
	return data_;
}


/*extracts slice from this block and copies to the given block, which must exist and have data allocated, Returns dest*/
Block::dataPtr Block::extract_slice(int rank, offset_array_t& offsets,
		BlockPtr destination) {
//	std::cout << "in extract_slice, printing destination block " << std::endl << *destination << std::endl;
	//extents = shape_.segment_sizes_;
	/*  subroutine tensor_block_slice_(nthreads,dim_num,tens,tens_ext,slice,slice_ext,ext_beg,ierr) bind(c,name='tensor_block_slice__') !PARALLEL
	 !This subroutine extracts a slice from a tensor block.
	 !INPUT:
	 ! - nthreads - number of threads requested;
	 ! - dim_num - number of tensor dimensions;
	 ! - tens(0:) - tensor block (array);
	 ! - tens_ext(1:dim_num) - dimension extents for <tens>;
	 ! - slice_ext(1:dim_num) - dimension extents for <slice>;
	 ! - ext_beg(1:dim_num) - beginning dimension offsets for <tens> (numeration starts at 0);
	 !OUTPUT:
	 ! - slice(0:) - slice (array);
	 ! - ierr - error code (0:success).
	 */
	int nthreads = sip::MAX_OMP_THREADS;
	int ierr = 0;

	sip::check(destination->data_ != NULL, "when trying to extract slice of a block, destination is NULL");
	sip::check(data_ != NULL, "when trying to extract slice of a block, source is NULL");

	tensor_block_slice__(nthreads, rank, data_, shape_.segment_sizes_,
			destination->data_, destination->shape_.segment_sizes_, offsets,
			ierr);
	sip::check(ierr == 0, "error value returned from tensor_block_slice__");
//	std::cout << "in extract_slice, after tensor_block_slice  printing destination block " << std::endl << *destination << std::endl;
	return destination->data_;
}

void Block::insert_slice(int rank, offset_array_t& offsets, BlockPtr source){
	/**subroutine tensor_block_insert_(nthreads,dim_num,tens,tens_ext,slice,slice_ext,ext_beg,ierr) bind(c,name='tensor_block_insert__') !PARALLEL
	!This subroutine inserts a slice into a tensor block.
	!INPUT:
	! - nthreads - number of threads requested;
	! - dim_num - number of tensor dimensions;
	! - tens_ext(1:dim_num) - dimension extents for <tens>;
	! - slice(0:) - slice (array);
	! - slice_ext(1:dim_num) - dimension extents for <slice>;
	! - ext_beg(1:dim_num) - beginning dimension offsets for <tens> (numeration starts at 0);
	!OUTPUT:
	! - tens(0:) - tensor block (array);
	! - ierr - error code (0:success).
	subroutine tensor_block_insert_(nthreads,dim_num,tens,tens_ext,slice,slice_ext,ext_beg,ierr)
	void tensor_block_insert__(int&, int&, double*, int*, double*, int*, int*, int&);
	*/
//	std::cout << "Block::insert_slice\n";
//	std::cout << "rank, offsets, source = " << rank << ", [" ;
//	for (int i = 0; i < MAX_RANK; ++i){
//		std::cout << (i ==0 ? "" : ",") << offsets[i];
//	}
//	std::cout << "]\n";
//	std::cout << *source << std::endl;
	int nthreads = sip::MAX_OMP_THREADS;
	int ierr = 0;
//	std::cout << "nthreads, rank, data_, " <<
//			"shape_.segment_sizes_, " <<
//			"source->data_, source->shape>_.segment_sizes_" << std::endl;
//	std::cout << nthreads << "," << rank << "," << data_[0] << "," << data_[1] << ",\n[" ;
//	for (int i = 0; i < MAX_RANK; ++i){
//		std::cout << (i ==0 ? "" : ",") << shape_.segment_sizes_[i];
//	}
//	std::cout << "]\n";
//	std::cout << source->data_[0] << "," << source->data_[1] << ",\n[" ;
//	for (int i = 0; i < MAX_RANK; ++i){
//		std::cout << (i ==0 ? "" : ",") << source->shape_.segment_sizes_[i];
//	}
//	std::cout << ']' << std::endl;
	tensor_block_insert__(nthreads, rank, data_, shape_.segment_sizes_,
			source->data_, source->shape_.segment_sizes_, offsets, ierr);
	sip::check(ierr==0, "error value returned from tensor_block_insert__");
}

std::ostream& operator<<(std::ostream& os, const Block& block) {
	os << "SHAPE=" << block.shape_ << std::endl;
	os << "SIZE=" << block.size_ << std::endl;
	int i = 0;
	int MAX_TO_PRINT = 800;
	int size = block.size_;
	int output_row_size = block.shape_.segment_sizes_[0];  //size of first dimension (really a column, but displayed as a row)
	if (block.data_ == NULL) {
		os << " data_ = NULL";
	} else {
		while (i < size && i < MAX_TO_PRINT) {

			for (int j = 0; j < output_row_size && i < size; ++j) {
				os << block.data_[i++] << "  ";

			}
			os << '\n';
		}
	}
	os << "END OF BLOCK" << std::endl;
	return os;
}

bool Block::operator==(const Block& rhs) const{
	if (this == &rhs) return true;
	return (size_ == rhs.size_) && (shape_ == rhs.shape_)
			&& std::equal(data_ + 0, data_+size_, rhs.data_);
}

void Block::free_host_data(){
	if (data_){
		delete [] data_;
	}
	data_ = NULL;
	status_[Block::onHost] = false;
	status_[Block::dirtyOnHost] = false;
}

void Block::allocate_host_data(){
	sip::check_and_warn(data_ == NULL, "Potentially causing a memory leak on host");
	data_ = new double[size_]();
	status_[Block::onHost] = true;
	status_[Block::dirtyOnHost] = false;
}


//char* Block::serialize(BlockPtr obj, int &size){
//	size = sizeof(obj->shape_) + sizeof(obj->size_) + obj->size_ * sizeof(dataPtr);
//	char * to_return = new char[size];
//
//	memcpy(to_return + 0, &obj->shape_, sizeof(obj->shape_));
//	memcpy(to_return + sizeof(obj->shape_), &obj->size_, sizeof(obj->size_));
//	memcpy(to_return + sizeof(obj->shape_) + sizeof(obj->size_), obj->data_, obj->size_ * sizeof(dataPtr));
//
//	return to_return;
//}

//Block::BlockPtr Block::deserialize(char* bytes, int size){
//	BlockShape shape_;
//    int size_;
//
//	memcpy(&shape_, bytes + 0, sizeof(shape_));
//	memcpy(&size_, bytes + sizeof(shape_), sizeof(size_));
//	dataPtr data_ = new double[size_];
//	memcpy(data_, bytes + sizeof(shape_) + sizeof(size_), size_ * sizeof(dataPtr));
//
//	sip::check(size == sizeof(shape_) + sizeof(size_) + size_ * sizeof(dataPtr),
//			"Deserialization of BlockPtr failed !");
//	sip::check(shape_.num_elems() == size_, "Incorrect size in deserialization of BlockPtr !");
//
//	Block * blk = new Block(shape_, data_);
//	return blk;
//
//}

/*********************************************************************/
/**						GPU Specific methods						**/
/*********************************************************************/
#ifdef HAVE_CUDA

Block::BlockPtr Block::new_gpu_block(BlockShape shape){
	BlockPtr block_ptr = new Block(shape, NULL);
	block_ptr->gpu_data_ = _gpu_allocate(block_ptr->size());
	block_ptr->status_[Block::onHost] = false;
	block_ptr->status_[Block::onGPU] = true;
	block_ptr->status_[Block::dirtyOnHost] = false;
	block_ptr->status_[Block::dirtyOnGPU] = false;
	return block_ptr;
}

Block::dataPtr Block::gpu_copy_data(BlockPtr source_block){
	dataPtr target = get_gpu_data();
	dataPtr source = source_block->get_gpu_data();
	int n = size();
	_gpu_device_to_device(target, source, n);
	return target;
}

void Block::free_gpu_data(){
	if (gpu_data_){
		_gpu_free(gpu_data_);
	}
	gpu_data_ = NULL;
	status_[Block::onGPU] = false;
	status_[Block::dirtyOnGPU] = false;
}

void Block::allocate_gpu_data(){
	sip::check_and_warn(gpu_data_ == NULL, "Potentially causing a memory leak on GPU");
	gpu_data_ = _gpu_allocate(size_);
	status_[Block::onGPU] = true;
	status_[Block::dirtyOnGPU] = false;
}


Block::dataPtr Block::get_gpu_data(){
	return gpu_data_;
}


Block::dataPtr Block::gpu_fill(double value){
	_gpu_double_memset(gpu_data_, value, size_);
	return gpu_data_;
}

Block::dataPtr Block::gpu_scale(double value){
	_gpu_selfmultiply(gpu_data_, value, size_);
	return gpu_data_;
}

#endif

/**
 * ROUTINES FOR SERVER Blocks
 *
 */
std::ostream& operator<<(std::ostream& os, const ServerBlock& block) {
	os << "SIZE=" << block.size_ << std::endl;
	int i = 0;
	int MAX_TO_PRINT = 800;
	int size = block.size_;
	int output_row_size = 30;  //size of first dimension (really a column, but displayed as a row)
	if (block.data_ == NULL) {
		os << " data_ = NULL";
	} else {
		while (i < size && i < MAX_TO_PRINT) {
			for (int j = 0; j < output_row_size && i < size; ++j) {
				os << block.data_[i++] << "  ";

			}
			os << '\n';
		}
	}
	os << "END OF SERVER BLOCK" << std::endl;
	return os;
}

} /* namespace sip */
