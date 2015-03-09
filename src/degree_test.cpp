// "Software"), to deal in the Software without restriction, including
// without limitation the rights (to use, copy, modify, merge, publish,
// distribute, sublicense, and/or sell copies of the Software, and to
// permit persons to whom the Software is furnished to do so, subject to
// the following conditions:
//
// The above copyright notice and this permission notice shall be included
// in all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
// OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
// MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
// NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE
// LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION
// OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
// WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
#include "ligra.h"
#include "gettime.h"
#include "math.h"
#include "parseCommandLine.h"
#include <xmmintrin.h>
#include <unistd.h>
using namespace std;


template <class vertex>
void iter_pagerank_syn(graph<vertex> GA,double* &p_curr,double* &p_next)
{
	
	int n_v=GA.n;
	int n_e=GA.m;


	vertex *G = GA.V;
	const double damping = 0.85; //alpha
	const double constant=(1-damping)*(1/(double)n_v); //constant
	
	int head=0;
	int tail=1024;
//	while(head<n_v)
//	{
//		if(tail>n_v)
//			tail=n_v;
//parallel_for(int i=head;i<tail;i++)
		parallel_for(int i=0;i<n_v;i++)
		{
			int in_degree=G[i].getInDegree();
			//int * in_edge=G.V[i].in_edge;
			double sum=0;
			for(int j=0;j<in_degree;j++) //gather
			{
				sum=sum+p_curr[2*G[i].getInNeighbor(j)]/p_curr[2*G[i].getInNeighbor(j)+1];
				//sum=sum+p_curr[G[i].getInNeighbor(j)];
			}
			p_next[2*i]=constant+damping*sum; //update
			
		}
		head=tail;
		tail=tail*2;
	//}
	//set zero and swap
	parallel_for(int i=0;i<n_v;i++) 
		p_curr[2*i] = 0;
	double* temp=p_curr;
	p_curr=p_next;
	p_next=temp;
}

template <class vertex>
void PageRank(graph<vertex> GA)
{
	
	int n_v=GA.n;
	vertex *G = GA.V;
	const double epsilon = 0.0000001;
	double one_over_n = 1/(double)n_v;
    double* p_curr = new double[2*n_v];
	double* p_next = new double[2*n_v];
	for(int i=0;i<n_v;i++)
	{
		p_curr[2*i] = one_over_n;
		p_next[2*i] = 0;
		p_curr[2*i+1] = (double)G[i].getOutDegree();
		p_next[2*i+1]=p_curr[2*i+1];	
	}
	
	for(int i=0;i<16;i++)
	{
		iter_pagerank_syn(GA,p_curr,p_next);
		cout<<"iteration:"<<i<<"complete!"<<endl;
	}
	delete[] p_curr;
	delete[] p_next;
}

template <class vertex>
void memtest(graph<vertex> GA)
{
	int n_v=GA.n;
	int n_e=GA.m;
	vertex *G = GA.V;
	double* p_curr = new double[n_v];
	double sum=0;
	int iter=128;
/*	for(int i=0;i<n_v;i++)
	{
		p_curr[i]=i;	
	}
	while(iter--)
	{
		for(int i=0;i<n_v;i++)
		{
			sum=sum+p_curr[i];	
		}
	}
	cout<<sum<<endl;
	 sleep(1);
	iter=8;
	while(iter--)
	{
		for(int i=0;i<n_v;i++)
		{
			int in_degree=G[i].getInDegree();
			for(int j=0;j<in_degree;j++) //gather
			{
				sum=sum+p_curr[G[i].getInNeighbor(j)];
			}
		}
	}
	cout<<sum<<endl;
	 */
	 
	/* 
	int p=2048;
	int * degree_count=new int[p];
	int total_degree=0;
	for(int k=0;k<p;k++)
	{
		degree_count[p]=0;
		//sleep(1);
		iter=1;
		while(iter--)
		{
			for(int i=k*(n_v/p);i<(k+1)*(n_v/p);i++)
			{
				int in_degree=G[i].getInDegree();
				degree_count[p]=degree_count[p]+in_degree;
				for(int j=0;j<in_degree;j++) //gather
				{
					sum=sum+p_curr[G[i].getInNeighbor(j)];
				}
			}
		}
		//cout<<"degree_count["<<k<<"]="<<degree_count[p]<<endl;
		cout<<degree_count[p]<<",";
		total_degree=total_degree+degree_count[p];
	}
	cout<<"total_degree="<<total_degree<<endl;
	sleep(1);
	cout<<endl;
	*/
/*	
	cout<<sum<<endl;
	sleep(1);

	
	
	int * p_e=new int[n_e];
	for(int i=0;i<n_e;i++)
	{
		p_e[i]=i%n_v;	
	}
	iter=8;
	sleep(1);
	while(iter--)
	{
		
		for(int i=0;i<n_e;i++)
		{
			sum=sum+p_curr[p_e[i]];
		}
	}
	 */
	iter=16;
//	sleep(1);
	const double damping = 0.85; //alpha
	const double constant=(1-damping)*(1/(double)n_v); //constant
	while(iter--)
	{
	
		for(int i=0;i<n_e;i++)
		{
			sum=sum+p_curr[GA.allocatedInplace[i]];
		}
	/*	
			for(int i=0;i<n_v;i++)
		{
			int in_degree=G[i].getInDegree();
			for(int j=0;j<in_degree;j++) //gather
			{
				sum=sum+p_curr[G[i].getInNeighbor(j)];
			}
			p_curr[i]=constant+damping*sum; //update
		}
		 */
	}
//	sleep(1);
	cout<<sum<<endl;
	 
}

template <class vertex>
void degree_count(graph<vertex> GA,double percent)
{
	
	int n_v=GA.n;
	int n_e=GA.m;
	vertex *G = GA.V;
	int n_vh=n_v*percent;// #vertices with high degree
	int n_eh_all=0; //#all edges connected to vertices with high degree
	int n_eh_intra=0; //#edges that both end points connected to vertices with high degree
	for(int i=0;i<n_vh;i++)
	{
		int in_degree=G[i].getInDegree();
		n_eh_all+=in_degree;
		for(int j=0;j<in_degree;j++) //gather
		{
			if(G[i].getInNeighbor(j)<n_vh)
				n_eh_intra++;
		}
	}
	//cout<<"n_v:"<<n_v<<" percent:"<<percent<<" n_vh:"<<n_vh<<" n_e:"<<n_e<<" n_eh_all:"<<n_eh_all
	//<<"("<<(double)n_eh_all/(double)n_e<<") n_eh_intra:"<<n_eh_intra<<"("<<(double)n_eh_intra/(double)n_e<<")"<<endl;
	cout<<","<<(double)n_eh_all/(double)n_e<<","<<(double)n_eh_intra/(double)n_e;
}

const char * split2filename(const char * path)
{
	const char * filename=path;
	while(*path!='\0')
	{
		if(*path=='/')
			filename=path+1;
		path++;	
	}
	return filename;
		
}

int parallel_main(int argc, char* argv[]) { 
	
	//cout<<"111111111"<<endl;
  commandLine P(argc,argv," [-s] <inFile>");
  char* iFile = P.getArgument(0);
  bool symmetric = P.getOptionValue("-s");
  bool binary = P.getOptionValue("-b");
  long rounds = P.getOptionLongValue("-rounds",3);
 if(symmetric) 
 {
    graph<symmetricVertex> G = 
      readGraph<symmetricVertex>(iFile,symmetric,binary); 
	//G.print_graph();
	//G.BFS_mapping();
	//G.ID_remapping();
	//G.print_graph();
	//PageRank(G);
//	cout<<"vertex_num="<<G.n<<endl<<"edge_num="<<G.m<<endl;
//	G.random_mapping();
//	G.ID_remapping();
//	memtest(G);
//	G.BFS_mapping();
//	G.ID_remapping();
//	memtest(G);
	G.degree_mapping();
//	G.ID_remapping();
	
//	memtest(G);
//	G.partition_mapping(262144);
	G.ID_remapping();
	double percent=0.0001;
	cout<<split2filename(iFile)<<","<<G.n<<","<<G.m;
	for(int i=0;i<4;i++)
	{	
		degree_count(G,percent);
		percent=percent*10;
	}
	degree_count(G,0.5);
	cout<<endl;
//	PageRank(G); 
//	PageRank(G); 
    G.del();
 }
 else
 {	 
    graph<asymmetricVertex> G = 
      readGraph<asymmetricVertex>(iFile,symmetric,binary); //asymmetric graph
	cout<<"vertex_num="<<G.n<<endl<<"edge_num="<<G.m<<endl;
//	G.random_mapping();
//	G.ID_remapping();
//	memtest(G);
	G.BFS_mapping();
//	G.ID_remapping();
//	memtest(G);
//	G.degree_mapping();
//	G.ID_remapping();
//	memtest(G);
//	G.partition_mapping(262144);
	G.ID_remapping();
	PageRank(G); 
	PageRank(G); 
 
    G.del();
 }

	
    
  
}
