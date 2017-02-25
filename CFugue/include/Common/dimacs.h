#ifndef _DIMACS__9FD17A0C_743B_4438_B8D7_9CD297CED5D3_
#define _DIMACS__9FD17A0C_743B_4438_B8D7_9CD297CED5D3_

#include "_TChar.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

struct dummy_listener
{
    inline static void OnProblemInformationRead(int nVertices, int nEdges) { }
    inline static void OnFoundEdge(int nVert1, int nVert2) { }
	inline static void OnLoadComplete() { }
};

/// Gets Nr_vert and Nr_edge from the preamble string
///    containing Dimacs format "p ??? num num"
int get_params(int& Nr_vert, int& Nr_edges, const char* Preamble)
{
	char c, *tmp;
	const char * pp = Preamble;
	int stop = 0;
	tmp = (char *)calloc(100, sizeof(char));
	
	Nr_vert = Nr_edges = 0;
	
	while (!stop && (c = *pp++) != '\0'){
		switch (c)
		  {
			case 'c':
			  while ((c = *pp++) != '\n' && c != '\0');
			  break;
			  
			case 'p':
			  sscanf(pp, "%s %d %d\n", tmp, &Nr_vert, &Nr_edges);
			  stop = 1;
			  break;
			  
			default:
			  break;
		  }
	}
	
	free(tmp);
	
	if (Nr_vert == 0 || Nr_edges == 0)
	  return 0;  /* error */
	else
	  return 1;
	
}

///////////////////////////////////////////////////////////
///
#define MAX_NR_VERTICES		10000
#define MAX_NR_VERTICESdiv8	(MAX_NR_VERTICES/8)
#define MAX_PREAMBLE 10000
///
///////////////////////////////////////////////////////////
// Retruns if there is edge in the binary bitmap loaded from binary input file
bool get_edge(int i, int j, char Bitmap[MAX_NR_VERTICES][MAX_NR_VERTICESdiv8])
{
    static const unsigned char masks[ 8 ] = { 0x01, 0x02, 0x04, 0x08, 0x10, 0x20, 0x40, 0x80 };

	int byte, bit;
	unsigned char mask;
	
	bit  = 7-(j & 0x00000007);
	byte = j >> 3;
	
	mask = masks[bit];
	return( (Bitmap[i][byte] & mask)==mask );
}

// Loads a binary input file and reports the found edges
template<class _listener>
bool read_graph_DIMACS_bin(const char* lpszFile)
{
    char Preamble[MAX_PREAMBLE];
    char Bitmap[MAX_NR_VERTICES][MAX_NR_VERTICESdiv8];

    int i, j, length = 0;
	
	FILE *fp = fopen(lpszFile, "r");
	if(fp == NULL) { fprintf(stderr, "\nERROR: Unable to open %s \n", lpszFile); return false; }

    // Try reading the length of the Preamble
	if (!fscanf(fp, "%d\n", &length))
	  { fprintf(stderr,"\nERROR: Corrupted preamble in %s \n", lpszFile); return false; }

	if(length >= MAX_PREAMBLE)
	  { fprintf(stderr,"\nERROR: Too long preamble in %s \n", lpszFile); return false; }

    // Read the Preamble itself
	fread(Preamble, 1, length, fp);
	Preamble[length] = '\0';
	
    // Get the Edge and Vertex Information
    int Nr_vert, Nr_edges;
	if (!get_params(Nr_vert, Nr_edges, Preamble))
	  { fprintf(stderr,"\nERROR: Corrupted preamble in %s \n", lpszFile); return false; }

    // Report the Edge and Vertex information to the App
    _listener::OnProblemInformationRead(Nr_vert, Nr_edges);

    // Read the Adjacency into a binary bitmap
	for ( i = 0
		 ; i < Nr_vert && fread(Bitmap[i], 1, (int)((i + 8)/8), fp)
		 ; i++ );

	fclose(fp);

    // Now report the Edges found in the bitmap
	for ( i = 0; i<Nr_vert; i++ )	  
		  for ( j=0; j<=i; j++ )
			if ( get_edge(i,j,Bitmap) ) 
                _listener::OnFoundEdge(i+1, j+1);  // Report the Edge to the App for processing	  

	_listener::OnLoadComplete(); // Report the completion of loading the graph

	return true;
}

// Loads an Ascii input file and reports the found edges
template<class _listener>
bool read_graph_DIMACS_ascii(const char* lpszFile)     
{
    char Preamble[MAX_PREAMBLE];

	int c, oc;
	char * pp = Preamble;
	int i,j;	
    int Nr_vert, Nr_edges;
	
	FILE *fp = fopen(lpszFile, "r");
	if(fp == NULL) { fprintf(stderr, "\nERROR: Unable to open %s \n", lpszFile); return false; }

    // Read past the Comments and Preamble section
	for(oc = '\0' ;(c = fgetc(fp)) != EOF && (oc != '\n' || c != 'e')
		; oc = *pp++ = c);
 
	ungetc(c, fp); 
	*pp = '\0';
	if(!get_params(Nr_vert, Nr_edges, Preamble)) // Get the Edge and Vertex Information
	  { fprintf(stderr,"\nERROR: Corrupted preamble in %s \n", lpszFile); return false; }

    // Report the Edge and Vertex information to the App
    _listener::OnProblemInformationRead(Nr_vert, Nr_edges);

    // Read the Edge Adjacency Information
	while ((c = fgetc(fp)) != EOF)
    {
		switch (c)
		  {
			case 'e':
                {
		          if (!fscanf(fp, "%d %d", &i, &j))
				  { fprintf(stderr, "\nERROR: corrupted inputfile %s \n", lpszFile); return false; }
		          
			        _listener::OnFoundEdge(i, j); // Report the Found Edge to the App
		          break;			  
                }
			case '\n':			  
			default:  break;
		  }
	}
	
	fclose(fp);
	
	_listener::OnLoadComplete(); // Report the completion of loading the graph

	return true;
}

// loads the given DIMACS graph file (binary / ascii)
template<class _listener>
bool load_DIMACS_graphfile(const char* lpszFile)
{
    if(strstr(lpszFile, ".b"))  // if the input file is Binary
        return read_graph_DIMACS_bin<_listener>(lpszFile);
                     
	// else consider it as an Ascii
	return read_graph_DIMACS_ascii<_listener>(lpszFile);
}

#endif // _DIMACS__9FD17A0C_743B_4438_B8D7_9CD297CED5D3_
