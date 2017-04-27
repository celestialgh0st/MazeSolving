/*
This program accepts an image of maze creates it's copy and **tries to solve that maze
Created by Rafid,piyush & Aniket
*/

//todo: find a way to visit=0 in struct and i.e how to initialize values in struct

#include <stdio.h>
#include <stdlib.h>
#include "png.h"
#include <string.h>
#include <stdbool.h>
#include <limits.h>
#define TOP 2
#define LEFT 1
#define RIGHT 3
#define BOTTOM 4
#define CHECK_LEFT ((*((g+i*width)+j-8)))
#define CHECK_RIGHT ((*((g+i*width)+j+8)))
#define CHECK_TOP (*((g+(i-8)*width)+j))
#define CHECK_BOTTOM ((*((g+(i+8)*width)+j)))
#define CHECK_CURRENT (*((g+i*width)+j))
#define INIT_VISIT temp->visit=0
//css


int height,width;
png_struct *png_ptr;//These two are mandatory structs
png_info  *info_ptr;//and must be declared in order to open a png image
png_byte color_type;//png_byte is not a struct but maybe just a typedef|o|
png_byte **row_pointers;
int number_of_passes;
int num_nodes;
png_byte bit_depth;
png_byte compression_type;
png_byte filter_method;

png_color *palette;
int number_palette;

//structure for creating nodes in an png
struct node{
	int x,y;
	struct node* left;
	struct node* top;
	struct node* right;
	struct node* bottom;
	
	int edge_weight[4];

	int visit;
	int node_num;

	struct node* prev;
	struct node* next;

};

//structure for using Dijkstra
struct stack{
		struct node* cur;

		struct stack* nxt;
		struct stack* prv;
};


struct node* start;
struct node* end;

struct stack* top;
struct stack* bottom;


//struct queue* find_queue_element(struct node* tmp);
//void store_in_queue(struct node* temp);
void graph_png();
bool check_pop_cond(struct stack*);
void printstack(struct stack*);
struct node* find_node(int , struct node*);
void weight_graph();
struct node* find_appr_min_weight_neighbour(struct node* temp);
void bfs(struct node* s);

typedef unsigned char uch;
//This function creates a copy of the orignal image
void create_copy(char *src, char *dest)
{
	int i, length;
	FILE *img,*img_cpy;
	img = fopen(src,"rb");
	img_cpy = fopen(dest,"wb");
	//finding the total lenght of file in bytes
	fseek(img,0,SEEK_END);
	length = ftell(img);
	fseek(img,0,SEEK_SET);


	for(i=0;i<length;i++)
	{

		fputc(fgetc(img),img_cpy);//Why does this work??!!

	}
	

	fclose(img);
	fclose(img_cpy);
	
}

int readpng_init(char *png)
{
	int x,y,y1,y2,x1,x2;
	char ch;
	int png_plte_ret;
	FILE *pngimg = fopen(png,"rb");
	uch sig[8];
	fread(sig,1,8,pngimg);
	/*
	from man page of fread()
	size_t fread(void *ptr, size_t size, size_t nmemb, FILE *stream);
	

	The  function  fread() reads nmemb items of data, each size bytes long,
    from the stream pointed to by stream,  storing  them  at  the  location
    given by ptr.
	*/
	if(png_sig_cmp(sig,0,8) != 0) //png_sig_cmp() returns 1 when png signature has matched
		return 1;

	printf("[DEBUG]Completed signature test");
	png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING,NULL,NULL,NULL);
	if(!png_ptr)
		return 4; //OUT OF MEMORY
	
	info_ptr = png_create_info_struct(png_ptr);
	if(!info_ptr)
		return 4; //OUT OF MEMORY
	
	printf("[DEBUG]Created info and image ptr\n");

	png_init_io(png_ptr,pngimg);
	printf("[DEBUG]stored pngimg(fp) inside the png_ptr struct\n");
	
	png_set_sig_bytes(png_ptr,8);


	png_read_info(png_ptr, info_ptr);

	//png_plte_ret=png_get_PLTE(png_ptr,info_ptr,&palette,&number_palette);
	height = png_get_image_height(png_ptr,info_ptr);
	width = png_get_image_width(png_ptr,info_ptr);
	bool graph[height][width];
	//png_set_palette_to_rgb(png_ptr);
	color_type = png_get_color_type(png_ptr, info_ptr);
	bit_depth = png_get_bit_depth(png_ptr,info_ptr);
	 printf("\n bit_depth=%d",bit_depth);
	printf("png_byte:%d\n",color_type);
	compression_type = png_get_compression_type(png_ptr,info_ptr);
	filter_method    = png_get_filter_type(png_ptr,info_ptr);

	ch = png_get_interlace_type(png_ptr,info_ptr);
	//printf("%d",ch);

	number_of_passes = png_set_interlace_handling(png_ptr);
	
    png_read_update_info(png_ptr, info_ptr);



    printf("[DEBUG]Interlace Handling successfull\n");

    row_pointers = (png_bytep*) malloc(sizeof(png_bytep) * height);
    for (y=0; y<height; y++)
        row_pointers[y] = (png_byte*) malloc(png_get_rowbytes(png_ptr,info_ptr));


    png_read_image(png_ptr, row_pointers);

    printf("[DEBUG]Stored data in row pointer\n");

    png_read_end(png_ptr,NULL);

    for (y=0; y<height; y++) {
                png_byte* row = row_pointers[y];
                for (x=0; x<width; x++) {
                        png_byte* ptr = &(row[x*4]);
                        if(ptr[0]==0)
                        	graph[y][x] = 0;
                        else
                        	graph[y][x] = 1;
                }
    }

    for (int x = 0; x < height; x++)
    {
    	for (int y = 0; y < width; ++y)
    	{
    		printf("%d",graph[x][y]);
    	}
    	printf("\n");
    }

    graph_png((bool * )graph);
    bfs(start);
    //zero out g

    for(x=0;x<height;x++)
    	for(y=0;y<width;y++)
    		graph[x][y] = 0;

    struct stack* temp1,*temp2;
    temp1 = bottom;
    temp2 = bottom->nxt;

    while(temp1 != top)
    {
    	if(temp1->cur->x == temp2->cur->x)
    	{
    		y1 = (temp1->cur->y > temp2->cur->y)?temp1->cur->y:temp2->cur->y;
    		y2 = (y1==temp1->cur->y)?temp2->cur->y:temp1->cur->y;
    		while(y2 != y1+1)
    		{
    			graph[temp1->cur->x][y2]=1;
    			y2++;
    		}
    	}
    	else
    	{
    		x1=(temp1->cur->x > temp2->cur->x)?temp1->cur->x:temp2->cur->x;
    		x2 = (x1 == temp1->cur->x)?temp2->cur->x:temp1->cur->x;
    		while(x2!=x1+1)
    		{
    			graph[x2][temp1->cur->y] = 1;
    			x2++;
    		}
    	}
    	temp1 = temp1->nxt;
    	temp2 = temp2->nxt;
    }
    for(x=0;x<height;x++){
    	for(y=0;y<width;y++){
    		printf("%d",graph[x][y]);
    	}
    	printf("\n");
    }

    for (y=0; y<height; y++) {
                png_byte* row = row_pointers[y];
                for (x=0; x<width; x++) {
                        png_byte* ptr = &(row[x*4]);
                        if(graph[y][x]==1)
                        {
                        	ptr[0]=255;
                        	ptr[1]=81;
                        	ptr[2]=0;
                        	
                        }
                }
    }


   /* for(y=0; y< height;y++)
    	free(row_pointers[y]);
    free(row_pointers);
	*/
	return 0;

}

void write_png_file(char* file_name)
{
	int y;
    FILE *fp = fopen(file_name, "wb");
              


        /* initialize stuff */
        png_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);


        info_ptr = png_create_info_struct(png_ptr);
       
        printf("\n[DEBUG] INITIALIZED INFO AND PNG PTR");
        png_init_io(png_ptr, fp);


   

        png_set_IHDR(png_ptr, info_ptr, width, height,
                     bit_depth, color_type, PNG_INTERLACE_NONE,
                     PNG_COMPRESSION_TYPE_BASE, PNG_FILTER_TYPE_BASE);

        png_write_info(png_ptr, info_ptr);


        /* write bytes */
   

        png_write_image(png_ptr, row_pointers);


        

        png_write_end(png_ptr, NULL);

        /* cleanup heap allocation */
        for (y=0; y<height; y++)
                free(row_pointers[y]);
        free(row_pointers);


        fclose(fp);
}



void bfs(struct node* s)
{
	struct stack* temp;
	temp = malloc(sizeof(struct stack));
	
	//first node
	printf("\n[DEBUG]trying to push first node on stack");
	temp->cur = s;
	temp->cur->visit = 1;
	temp->prv = temp->nxt = NULL;
	bottom=top= temp;
	printf("\n[DEBUG] pushed first node on stack"); 
	

	while(temp->cur != end)
	{
		if(check_pop_cond(temp))//(temp->cur->left != NULL && temp->cur->left->visit==1) && (temp->cur->top != NULL && temp->cur->top->visit==1) && (temp->cur->right != NULL && temp->cur->right->visit==1) && (temp->cur->bottom!=NULL && temp->cur->bottom->visit==1))
		{	//pop
			printf("\n[DEBUG]Popping node (%d,%d)",temp->cur->x,temp->cur->y);
			temp = temp->prv;
			free(temp->nxt);
			temp->nxt = NULL;
			top = temp;
			//printstack(top);
		}
		else{
			struct node* n= find_appr_min_weight_neighbour(temp->cur);
			//push
			printf("\n[DEBUG] PUSHING node (%d,%d) on stack..",n->x,n->y);
			temp->nxt = malloc(sizeof(struct stack));
			temp->nxt->prv = temp;
			temp = temp->nxt;
			temp->nxt=NULL;
			top = temp;
			temp->cur = n;
			n->visit = 1;
			printf("\n[DEBUG] PUSH node (%d,%d) on stack..",n->x,n->y);
			//printstack(top);
		}
		temp = top;
	}
	//printstack(bottom);
}

bool check_pop_cond(struct stack* temp)
{
	struct node* array[4];
	int i=0,j;
	bool r=1;

	if(temp->cur->left!=NULL)
	{
		array[i] = temp->cur->left;
		i++;
	}
	if(temp->cur->top!=NULL)
	{
		array[i] = temp->cur->top;
		i++;
	}
	if(temp->cur->right!=NULL)
	{
		array[i] = temp->cur->right;
		i++;
	}
	if(temp->cur->bottom!=NULL)
	{
		array[i] = temp->cur->bottom;
		i++;
	}


	for(j=0;j<i;j++)
	{
		r = r && array[j]->visit;
	}

	return r;
}



void printstack(struct stack* temp){
	do{
		printf("%d,%d\n",temp->cur->x,temp->cur->y);
		temp=temp->nxt;
	}while(temp!= bottom);

}

struct node* find_appr_min_weight_neighbour(struct node* temp)
{
	int i,t=INT_MAX;
	if(t>((temp->left == NULL)?INT_MAX:temp->left->edge_weight[0]) && temp->left->visit==0){
		t = (temp->left == NULL)?INT_MAX:temp->left->edge_weight[0];
		i=0;
	}
	if(t>((temp->top == NULL)?INT_MAX:temp->top->edge_weight[1]) && temp->top->visit==0){
		t = (temp->top == NULL)?INT_MAX:temp->top->edge_weight[1];
		i=1;
	}
	if(t>((temp->right == NULL)?INT_MAX:temp->right->edge_weight[2]) && temp->right->visit==0){
		t = (temp->right == NULL)?INT_MAX:temp->right->edge_weight[2];
		i=2;
	}
	if(t>((temp->bottom == NULL)?INT_MAX:temp->bottom->edge_weight[3]) && temp->bottom->visit==0){
		t = (temp->bottom == NULL)?INT_MAX:temp->bottom->edge_weight[3];
		i=3;
	}
	if(i==0)
		return temp->left;
	else if(i==1)
		return temp->top;
	else if(i==2)
		return temp->right;
	else
		return temp->bottom;
}








void graph_png(bool *g)
{
	int i,j,n=0;
	printf("[DEBUG] Inside graph png\n");
	struct node* temp;
	for(i=0;i<height;i=i+8)
	{
		printf("i=%d\n\n",i);
		for(j=0;j<width;j+=8)
		{
		
			if(i==0)
			{
				
				while(!(*((g+i*width)+j)))
				{
					printf("[Debug] No problem in 2-d array %d: \n",(*((g+i*width)+j)));
					j++;
				}
				j=j+6;
				printf("[DEBUG] Trying to allocate memory for struct\n");
				temp=malloc(sizeof (struct node));
				printf("[DEBUG] allocated memory for first struct");
				printf("[DEBUG]Creating the first node at position (%d,%d)\n",i,j);
				temp->prev=NULL;
				temp->node_num = n;
				n++;
				temp->x=i;
				temp->y=j;
				INIT_VISIT;
				temp->left=temp->right=temp->bottom=NULL;
				start = temp;
				break;
			}
			else if(i==height-2)
			{
				while(!(*((g+i*width)+j)))
					j++;
				j=j+6;
				temp->node_num = n;
				n++;
				printf("[DEBUG]Creating last node\n");
				temp->next =(struct node*)malloc(sizeof(struct node));
				temp->next->prev = temp;
				temp = temp->next;
				
				temp->x=i;
				temp->y=j;

				
				 INIT_VISIT;
				temp-> top = find_node(TOP,temp);
				temp->right = temp->left=temp->bottom=NULL;
				end = temp;
				break;
			}
			else if(CHECK_CURRENT && ( (CHECK_LEFT + CHECK_RIGHT + CHECK_BOTTOM + CHECK_TOP == 3) || (!CHECK_LEFT + !CHECK_RIGHT + !CHECK_BOTTOM + !CHECK_TOP ==3) || ( (CHECK_LEFT && CHECK_BOTTOM) || (CHECK_LEFT && CHECK_TOP) || (CHECK_RIGHT && CHECK_BOTTOM) || (CHECK_RIGHT && CHECK_TOP) ))){

				printf("\ncurrent = %d",CHECK_CURRENT);
				printf("\tLeft = %d",CHECK_LEFT);
				printf("\tRIGHT = %d",CHECK_RIGHT);
				printf("\tBOTTOM = %d",CHECK_BOTTOM);
				printf("\tTOP = %d\n",CHECK_TOP);
				temp->next = (struct node*)malloc(sizeof(struct node));
					temp->next->prev = temp;
					temp= temp->next;
					//setting the coordinates
					temp->x = i;
					temp->y = j;
					INIT_VISIT;
					printf("[DEBUG]Creating an intermidiate node at position(%d,%d)\n",i,j);

					if(CHECK_TOP==1)
						temp->top =find_node(TOP,temp);
					if(CHECK_LEFT==1)
						temp->left =find_node(LEFT,temp);
					temp->node_num = n;
					n++;

			}
			else
			{
				printf("DID NOTHJING at %d %d\n",i,j);
			}
		}
		
	}
	printf("\n[DEBUG] NUMBER of nodes = %d ",n);
	num_nodes = n;
	weight_graph(n);

}



struct node *find_node(int side,struct node* tmp){
	int cor;
	struct node* tmp2 = tmp;
	if(side==1)//i.e if we need to find the left node
	{
		tmp->prev->right=tmp;
		return tmp->prev;
	}
	else//i.e we need to find the top node
	{
		cor = tmp->y;
		do{
			tmp=tmp->prev;
		}while(cor!=(tmp->y));
		tmp->bottom = tmp2;
	}
	return tmp;

}



void weight_graph(int num_of_nodes)
{
	struct node* temp= start;
	int i;
	for(i=0;i<num_of_nodes;i++)
	{
		if(temp == start)
		{
			temp->edge_weight[0]=0;
			temp->edge_weight[1]=0;
			temp->edge_weight[2]=0;
			temp->edge_weight[3]=1;
		}
		else
		{
			if(temp->top != NULL)
			{
				temp->edge_weight[1]=temp->x - temp->top->x;
			}
			if(temp->left != NULL)
			{
				temp->edge_weight[0] = temp->y - temp->left->y;
			}
			if(temp->right != NULL)
			{
				temp->edge_weight[2] = temp->right->y - temp->y;
			}
			if(temp->bottom != NULL)
			{
				temp->edge_weight[3] = temp->bottom->x - temp->x;
			}
		}
		temp=temp->next;
	}
	temp=start;
	for(i=0;i<num_of_nodes;i++)
	{
		printf("x,y = %d,%d\n",temp->x,temp->y);
		printf("edge_weight = %d,%d,%d,%d",temp->edge_weight[0],temp->edge_weight[1],temp->edge_weight[2],temp->edge_weight[3]);
		printf("current=%p\n", temp);
		printf("left = %p\n",temp->left);
		printf("top = %p\n", temp->top);
		printf("right = %p\n", temp->right);
		printf("bottom=%p\n\n", temp->bottom);
		temp=temp->next;
		printf("\n[DEBUG]");
		}
}





int main(int argc, char *argv[])
{
	int errorno;
	if(argc != 2)
	{
		printf("Usage:%s <path_to_image>",argv[0]);
		return 0;
	}

	char *des_file_name = (char *) malloc(sizeof(char)*strlen(argv[1]));
	char *src_file_name = (char *) malloc(sizeof(char)*(strlen(argv[1]) + 5));

	FILE *img_cpy;

	
	sprintf(des_file_name,"%s_soln",argv[1]);
	sprintf(src_file_name,"%s",argv[1]);
	
	create_copy(src_file_name,des_file_name);

	errorno=readpng_init(des_file_name);
	write_png_file(des_file_name);

	free(des_file_name);
	free(src_file_name);
	return 0;
}
