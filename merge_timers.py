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
#line_re = re.compile(r'([\+\.a-zA-Z0-9_-]*)\s*', re.I)  # Use findall())
line_re = re.compile(r'(\S*)\s*', re.I)  # Use findall())


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
def merge_files(files, output_name, prefix_of_file, size_of_header_columns_map):
    ''' Arguments
    files -- the list of files to create a csv for
    output_name -- the name of the output csv file
    prefix_of_file -- the prefix of the files to merge. Used to provide headers
    size_of_header_columns_map -- size(number) of header columns
    '''
    files.sort()    # Sort the list of file names
    
    data_map = {}           # "CounterTypeForProgram" => 3d table of data per rank
    header_column_map = {}  # "CounterTypeForProgram" => 2d table of data per rank
    header_row_map = {}     # "CounterTypeForProgram" => 1d table of data per rank - row header
    for fnum, fname in enumerate(files, start=0):
        with open(fname) as f:
            lines = f.readlines()
            sialx_prog = ""     # Name of the sial program being processed
            profile_type = ""   # Type of profile being read         
            matched_program_name = False # Whether currenly processed line contains program name
            line_after_program_name = False # Whether current line is the one after program name
    
            data_table = None
            header_column = None    
            header_row = None    
            skip_timers = False # Whether to skip including a particular type of timer

            # Process all lines in the file
            for line_number, l in enumerate(lines, start=0):
                
                # Match empty Line
                if l == '\n':   # empty lines indicate the end of a table
                    sialx_prog = None
                    profile_type = None
                    matched_program_name = False
                    line_after_program_name = False 
                    skip_timers = False
                    continue
                if skip_timers:
                    continue
                l = l.strip()
                # Match line containing program name    
                m_prog = program_re.match(l)
                if m_prog is not None:
                    profile_type = m_prog.group(1).strip() # Type of profile info
                    if size_of_header_columns_map[profile_type] < 0:
                        skip_timers = True
                        continue
                    sialx_prog = m_prog.group(2).strip()  # Name of sialx program
                    l = sialx_prog + ' ' + profile_type
                    data_table = data_map.get(l)
                    header_column = header_column_map.get(l)
                    header_row = header_row_map.get(l)
                    if (data_table is None):
                        data_table = []
                        data_map[l] = data_table
                        header_column = []
                        header_column_map[l] = header_column
                        header_row = []
                        header_row_map[l] = header_row
                        
                    
                    data_table.append([])   # For this fnum
                    assert len(data_table) == fnum + 1
                                                          
                    line_after_program_name = True
                    continue
                
                                                       
                size_of_header_column = size_of_header_columns_map[profile_type]
                #column_to_merge = column_to_write_map[profile_type]
                
                row = line_re.findall(l)
                row = filter(None, row)
                num_columns = len(row)
                
                
                 # Record table headers from file numbered 0
                if line_after_program_name:
                    line_after_program_name = False
                    if fnum == 0:
                        for i in row:
                            header_row.append(i.strip())
                        #print 'appended ' + row_header + ' to ' + str(line_number) + ' of data_table' 
                    continue
                
                # Recored column headers like PC, instruction opcode, etc
                if fnum == 0:      
                    this_column = []
                    #Append header columns (name of event, sialx line, pc, etc)  
                    for i in range(size_of_header_column):
                        this_column.append(row[i])
                    header_column.append(this_column)  
                    #print 'appended ' + row[i] + ' to ' + str(line_number) + ' of header_column' 

                             
                # Data line - time, counter, blockwait, etc
                this_file_data_row = []
                for i in range(size_of_header_column, num_columns):
                    this_file_data_row.append(row[i])
                    
                data_table[fnum].append(this_file_data_row)                 
                #print 'appended ' + row[column_to_merge] + ' to ' + str(line_number) + ' of data_table'
                                                       
        
    f = open(output_name, 'wb')
    try:
        writer = csv.writer(f)
        program_list = data_map.keys()
        program_list.sort()
        for p in program_list:
            header_row = header_row_map[p]
            header_column = header_column_map[p]
            
            # data[x] contains data from rank x
            # data[x][y] contains the data row y from rank x
            # data[x][y][z] is the zth data element of data row y from rank x
            data = data_map[p]
            
            header_row_of_header_columns = []
            len_header_row_of_header_columns = len(header_column[0])
            
            for i in range(len_header_row_of_header_columns):# write out len(header_column_map of header_row)
                header_row_of_header_columns.append(header_row[i])
    
            # for each of the data columns, choose the appropriate data header
            for dc in range(len(data[0][0])):# Number of data elems in Worker/Server 0's row should be the same as others
                
                # Write out the type of counter and program name 
                writer.writerow([p])            
            
                # Write out the header row (Will look like so : Name | Count (0) | Count(1) | Count(2) |...
                header_row_to_write = header_row_of_header_columns[:]
                for d in range(len(data)):
                    base_file_name = os.path.basename(files[d])
                    rank = base_file_name.replace(prefix_of_file, '')
                    header_row_to_write += [header_row[len_header_row_of_header_columns + dc] + '('+ rank + ')']
                writer.writerow(header_row_to_write)
                
                # write out the data (including header column)
                for drow in range(len(data[0])):
                    data_to_write = []
                    for d in range(len(data)):
                        # Append column header when appending data for rank 0
                        if d == 0:
                            data_to_write.extend(header_column[drow])
                        data_to_write.append(data[d][drow][dc])
                    writer.writerow(data_to_write)
                #print data_to_write
                
            # write empty row       
                writer.writerow([]) 
            writer.writerow([])
       
    finally:
        f.close()
     
    print "Created file " + output_name
    
# MAIN

# Time Column
extract_column_map = {"SialxTimers":3,
                      "Timers":1, 
                      "Counters":1, 
                      "MaxCounters":1,
                      "ServerTimers":3,
                      "ProfileTimers":-1}
# Merge worker files
worker_files = glob.glob(prefix_dir + '/worker.profile.*')
merge_files(worker_files, 'worker_profile.csv', 'worker.profile.', extract_column_map)


# Merge server files
server_files = glob.glob(prefix_dir + '/server.profile.*')
merge_files(server_files, 'server_profile.csv', 'server.profile.', extract_column_map)
