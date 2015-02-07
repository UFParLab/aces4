#!/usr/bin/python
import re
import glob
import csv
import sys
import os
import time

# Program to merge worker.timer.* and server.timer.* in a directory
# and spit out a csv file.
# Assumes that all workers and servers print out profile data at the
# exact same line numbers for corresponding events

# USAGE:
# python /path/to/merge_timer.py /directory/of/profile/files
# Will create the csv in the current directory.


# GLOBALS
prefix_dir = "."        # Dir to find profile files
if len(sys.argv) > 1:
    prefix_dir = sys.argv[1]

program_re = re.compile( r'(.*)\s*for\s*Program\s*(.*)', re.I) # match()
line_re = re.compile(r'([\+\.a-zA-Z0-9_-]*)\s*', re.I)  # Use findall())


# Measures and prints time of a method using decorators.
# http://stackoverflow.com/questions/5478351/python-time-measure-function
def timing(f):
    def wrap(*args):
        time1 = time.time()
        ret = f(*args)
        time2 = time.time()
        print '%s function took %0.3f ms' % (f.func_name, (time2-time1)*1000.0)
        return ret
    return wrap

# Merges *.profile.* files and spits out a csv
@timing
def merge_files(files, output_name, prefix_of_file, column_to_write_map, header_columns_map):
    ''' Arguments
    files -- the list of files to create a csv for
    output_name -- the name of the output csv file
    prefix_of_file -- the prefix of the files to merge. Used to provide headers
    column_to_write_map -- index of the column to merge
    header_columns_map -- size(number) of header columns
    '''
    files.sort()    # Sort the list of file names
    data_table = []
    header_column = []
    for fnum, fname in enumerate(files, start=0):
        #print fname
        with open(fname) as f:
            lines = f.readlines()
            sialx_prog = ""     # Name of the sial program being processed
            profile_type = ""   # Type of profile being read         
            matched_program_name = False # Whether currenly processed line contains program name
            line_after_program_name = False # Whether current line is the one after program name
            
            # When encounter 1st file, allocate as many lists as there are lines.
            if fnum == 0:
                for i in range(len(lines)):
                    data_table.append([])
                    header_column.append([])
                    
            
            # Process all lines in the file
            for line_number, l in enumerate(lines, start=0):

                # Match empty Line
                if l == '\n':   # empty lines indicate the end of a table
                    sialx_prog = ""
                    profile_type = ""
                    matched_program_name = False
                    line_after_program_name = False 
                    continue
                
                # Match line containing program name    
                m_prog = program_re.match(l)
                if m_prog is not None:
                    profile_type = m_prog.group(1).strip() # Type of profile info
                    sialx_prog = m_prog.group(2).strip()  # Name of sialx program

                    # processing 1st file, add program name
                    if fnum == 0:                    
                    # Append program line
                        header_column[line_number].append(l.strip()) # Remove newline at the end
                        #print 'appended ' + l + ' to ' + str(line_number) + ' of header_column'
                    
                    line_after_program_name = True
                    continue
                
                                                       
                header_columns_length = header_columns_map[profile_type]
                column_to_merge = column_to_write_map[profile_type]
                
                row = line_re.findall(l)
                num_columns = len(row)
                
                # processing 1st file, add program name & header columns
                if fnum == 0:        
                    #Append header columns (name of event, sialx line, pc, etc)  
                    for i in range(header_columns_length):
                        header_column[line_number].append(row[i])
                        #print 'appended ' + row[i] + ' to ' + str(line_number) + ' of header_column' 

                # Add Row header (rank of worker / server)
                if line_after_program_name:
                    base_file_name = os.path.basename(fname)
                    row_header = base_file_name.replace(prefix_of_file, '')
                    data_table[line_number].append(row_header)
                    #print 'appended ' + row_header + ' to ' + str(line_number) + ' of data_table' 

                    line_after_program_name = False
                    
                # Data line - time, counter, blockwait, etc                 
                else:
                    data_table[line_number].append(row[column_to_merge])
                    #print 'appended ' + row[column_to_merge] + ' to ' + str(line_number) + ' of data_table'
                                                       
        
    f = open(output_name, 'wb')
    try:
        writer = csv.writer(f)
        assert len(data_table) == len(header_column)
        for i in range(len(data_table)):
            to_write_row = []
            to_write_row.extend(header_column[i])
            to_write_row.extend(data_table[i])
            writer.writerow(to_write_row)
    finally:
        f.close()
    
    print "Created file " + output_name


# MAIN

# Time Column
extract_column_map = {"SialxTimers":3,
                      "Timers":1, 
                      "Counters":1, 
                      "MaxCounters":1,
                      "ServerTimers":3}

extract_block_wait_map = {"SialxTimers":4,
                      "Timers":1, 
                      "Counters":1, 
                      "MaxCounters":1,
                      "ServerTimers":4}

extract_count_map = {"SialxTimers":5,
                      "Timers":1, 
                      "Counters":1, 
                      "MaxCounters":1,
                      "ServerTimers":7}
# Merge worker files
worker_files = glob.glob(prefix_dir + '/worker.profile.*')
merge_files(worker_files, 'worker_profile.csv', 'worker.profile.', extract_column_map, extract_column_map)
merge_files(worker_files, 'worker_blkwt.csv', 'worker.profile.', extract_block_wait_map, extract_column_map)
merge_files(worker_files, 'worker_count.csv', 'worker.profile.', extract_count_map, extract_column_map)


# Merge server files
server_files = glob.glob(prefix_dir + '/server.profile.*')
merge_files(server_files, 'server_profile.csv', 'server.profile.', extract_column_map, extract_column_map)
merge_files(server_files, 'server_blkwt.csv', 'server.profile.', extract_block_wait_map, extract_column_map)
merge_files(server_files, 'server_count.csv', 'server.profile.', extract_count_map, extract_column_map)


















