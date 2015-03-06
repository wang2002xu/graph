import sys
import re 



fin=open(sys.argv[1],'r')
v_list=[]
current_vid=-1
max_id=0
for line in fin.readlines():
	if line[0] != '#':
		v_info_line=re.split(r'\s+',line)
#		print v_info_line
		v_id=int(v_info_line[0])
		v_out_edge=int(v_info_line[1])
		if max_id < v_id:
			max_id=v_id
		if max_id < v_out_edge:
			max_id=v_out_edge
		if v_id != current_vid:
			while current_vid!=v_id:
				current_vid=current_vid+1
				v_list.append([])
		v_list[current_vid].append(v_out_edge)
while current_vid<max_id:
	current_vid=current_vid+1
	v_list.append([])
#print v_list
fin.close()	
empty_list=[]
for i in range(len(v_list)):
	if len(v_list[i]) == 0:
		empty_list.append(i);
print empty_list
empty_list_in=[0]*len(v_list)
#print len(empty_list_in)
in_edge_empty=[]
for edge_list in v_list:
	for out_edge in edge_list:
#		 print out_edge
		 empty_list_in[int(out_edge)]=1
#		 print empty_list_in
for i in range(len(empty_list_in)):
	if empty_list_in[i]==0:
		in_edge_empty.append(i)
print in_edge_empty


delete_list=[]
for i in range(len(v_list)):
#		print len(v_list[i])
#		print empty_list_in[i]
		if len(v_list[i])==0 and empty_list_in[i]==0:
			print("delet %d" % i)
#			del v_list[l]  
			delete_list.append(i)		


v_num=len(v_list)-len(delete_list)
e_num=0
for edge_list in v_list:
	e_num=e_num+len(edge_list)
print("v_num=%d" % (v_num))
print ("e_num=%d" % (e_num))
fout=open(sys.argv[2],'w')
fout.write("AdjacencyGraph\n")
fout.write("%d\n" % v_num)
fout.write("%d\n" % e_num)

v_align=[-1]*len(v_list)
align_offset=0
for i in range(len(v_list)):
	if len(delete_list)!=0 and  i == delete_list[0]:
		del delete_list[0]
		align_offset=align_offset+1
	else:
		v_align[i]=i-align_offset

v_list_align=[]
for i in range(len(v_list)):
	if v_align[i]!=-1:
		edge_list=[]
		for out_edge in v_list[i]:
#			print(out_edge)
			edge_list.append(v_align[out_edge])
		v_list_align.append(edge_list)

#print(v_list_align)	
current_offset=0
for i in range(len(v_list_align)):
	fout.write("%d\n" % current_offset)
	current_offset=current_offset+len(v_list[i])
for edge_list in v_list_align:
	w2fout=[str(line)+'\n' for line in edge_list]
	fout.writelines(w2fout)



fout.close()
	
