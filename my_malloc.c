/*
 * CS 2110 Fall 2018
 * Author:
 */

/* we need this for uintptr_t */
#include <stdint.h>
/* we need this for memcpy/memset */
#include <string.h>
/* we need this to print out stuff*/
#include <stdio.h>
/* we need this for the metadata_t struct and my_malloc_err enum definitions */
#include "my_malloc.h"
/* include this for any boolean methods */
#include <stdbool.h>

/*Function Headers
 * Here is a place to put all of your function headers
 * Remember to declare them as static
 */

/* Our freelist structure - our freelist is represented as two doubly linked lists
 * the address_list orders the free blocks in ascending address
 * the size_list orders the free blocks by size
 */

metadata_t *address_list;
metadata_t *size_list;

/* Set on every invocation of my_malloc()/my_free()/my_realloc()/
 * my_calloc() to indicate success or the type of failure. See
 * the definition of the my_malloc_err enum in my_malloc.h for details.
 * Similar to errno(3).
 */
enum my_malloc_err my_malloc_errno;

/* MALLOC
 * See my_malloc.h for documentation
 */
void *my_malloc(size_t size) {
	my_malloc_errno = NO_ERROR;
	if(size == 0){
		my_malloc_errno = NO_ERROR;
		printf("%s\n","hi" );
		return NULL;
	}
	int size_needed = size + TOTAL_METADATA_SIZE;
	if(size_needed > SBRK_SIZE){
		my_malloc_errno = SINGLE_REQUEST_TOO_LARGE;
		printf("%s\n","hi" );
		return NULL;
	}

	metadata_t *currentNode = size_list;
	metadata_t *currentNode_addr = address_list;
	metadata_t *another_node = address_list;

	currentNode = size_list;
	while(currentNode != NULL){
		
		if(currentNode -> size == (unsigned long)size_needed){
			while(currentNode_addr != NULL){
				if(currentNode -> size == currentNode_addr -> size){
					if(currentNode_addr -> prev_addr == NULL){
						if(currentNode_addr -> next_addr == NULL){
							address_list = NULL;
						} else{
							currentNode_addr -> next_addr -> prev_addr = NULL;
							address_list = currentNode_addr -> next_addr;
						}
					} else if(currentNode_addr -> next_addr == NULL){
						currentNode_addr -> prev_addr -> next_addr = NULL;
					} else{
						currentNode_addr -> prev_addr -> next_addr = currentNode_addr -> next_addr;
						currentNode_addr -> next_addr -> prev_addr = currentNode_addr -> prev_addr;
					}

					break;
				}
				currentNode_addr = currentNode_addr -> next_addr;
			}

			if(currentNode -> prev_size == NULL ){
				if(currentNode -> next_size == NULL){
					size_list = NULL;
				} else{
					currentNode -> next_size -> prev_size = NULL;
					size_list = currentNode -> next_size;
				}
			} else if(currentNode -> next_size == NULL){
					currentNode -> prev_size -> next_size = NULL;
			} else{
				currentNode -> prev_size -> next_size = currentNode -> next_size;
				currentNode -> next_size -> prev_size = currentNode -> prev_size;
			}
		

			currentNode -> canary = ((uintptr_t)currentNode ^ CANARY_MAGIC_NUMBER) + 1;
			currentNode -> next_addr = NULL;
			currentNode -> prev_addr = NULL;
			currentNode -> prev_size = NULL;
			currentNode -> next_size = NULL;
			*((unsigned long*)((uint8_t*)currentNode + size_needed - sizeof(unsigned long))) = currentNode -> canary;
			printf("%s\n","h" );
			return (uint8_t* ) currentNode  + sizeof(metadata_t);

		} 
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		else if(currentNode -> size >= (unsigned long)size_needed + (unsigned long)MIN_BLOCK_SIZE){
			metadata_t *split= (metadata_t*)((uint8_t*)currentNode + currentNode -> size - size_needed);
			split -> size =  size_needed;
			currentNode -> size = currentNode -> size - size_needed;			
			split -> canary = ((uintptr_t)split ^ CANARY_MAGIC_NUMBER) + 1;
			*((unsigned long*)((uint8_t*)split + size_needed - sizeof(unsigned long))) = split -> canary;
			if(currentNode -> prev_size == NULL)
				size_list = currentNode -> next_size;

			metadata_t *curr = size_list;

			if(currentNode -> prev_size != NULL || currentNode -> next_size != NULL){

				if(currentNode -> prev_size != NULL)
					currentNode -> prev_size -> next_size = currentNode -> next_size;
				if(currentNode -> next_size != NULL)
					currentNode -> next_size -> prev_size = currentNode -> prev_size;
				while(curr != NULL){

					if(curr -> prev_size == NULL && currentNode -> size <= curr -> size ){
					
						
						currentNode -> next_size = curr;
						currentNode -> prev_size = NULL;
						currentNode -> next_size -> prev_size = currentNode;

						size_list = currentNode;
						break;
					} else if(curr -> next_size == NULL && curr -> size < currentNode -> size){
						currentNode -> prev_size = curr;
						currentNode -> next_size = NULL;
						currentNode -> prev_size -> next_size = currentNode;
						break;
					}
					else if(currentNode -> size < curr -> size){

						currentNode -> next_size = curr;
						currentNode -> prev_size = curr -> prev_size;
						curr -> prev_size -> next_size = currentNode;
						currentNode -> next_size -> prev_size = currentNode;
						break;
					}
					curr = curr -> next_size;
					if(curr == NULL)
						break;
				} 
			}
			printf("%s\n","hi" );
			return (uint8_t*)split + sizeof(metadata_t);
			// split -> prev_size = currentNode -> prev_size;
			// split -> prev_addr = currentNode -> prev_addr;
			// split -> next_size = currentNode -> next_size;
			// split -> next_addr = currentNode -> next_addr;
		} else if(currentNode -> next_size == NULL){
			metadata_t *newblock =  my_sbrk(SBRK_SIZE);
			if(newblock == NULL){
				my_malloc_errno = OUT_OF_MEMORY;
				return NULL;
			}
			newblock -> size = SBRK_SIZE;
			newblock -> canary = ((uintptr_t)newblock ^ CANARY_MAGIC_NUMBER) + 1;
			printf("%p\t%p\t%p\n",(void *)another_node, (void*)newblock, (void*)((uint8_t*)another_node+another_node->size));
			printf("%d\n", (int)(size_needed));
			if((void*)((uint8_t *)another_node + another_node -> size)  == (void*)newblock){
				printf("newblock: %d\n", (int)size_needed);
				another_node -> size += newblock -> size;
				if(another_node -> prev_size == NULL)
				size_list = another_node -> next_size;
				if(another_node -> prev_size != NULL)
					another_node -> prev_size -> next_size = another_node -> next_size;
				if(another_node -> next_size != NULL)
					another_node -> next_size -> prev_size = another_node -> prev_size;

				metadata_t *curr = size_list;

				while(curr != NULL){

					if(curr -> prev_size == NULL && another_node -> size <= curr -> size ){
					
						
						another_node -> next_size = curr;
						another_node -> prev_size = NULL;
						another_node -> next_size -> prev_size = another_node;

						size_list = another_node;
						break;
					} else if(curr -> next_size == NULL && curr -> size < another_node -> size){
						another_node -> prev_size = curr;
						another_node -> next_size = NULL;
						another_node -> prev_size -> next_size = another_node;
						break;
					}
					else if(another_node -> size < curr -> size){

						another_node -> next_size = curr;
						another_node -> prev_size = curr -> prev_size;
						curr -> prev_size -> next_size = another_node;
						another_node -> next_size -> prev_size = another_node;
						break;
					}
					curr = curr -> next_size;
					if(curr == NULL)
						break;
				} 

			} else if(newblock + newblock -> size == another_node){
				newblock -> size  += newblock -> size;
			} else{
				metadata_t *curr = size_list;

			
				while(curr != NULL){

					if(curr -> prev_size == NULL && newblock -> size <= curr -> size ){
					
						
						newblock -> next_size = curr;
						newblock -> prev_size = NULL;
						newblock -> next_size -> prev_size = newblock;

						size_list = newblock;
						break;
					} else if(curr -> next_size == NULL && curr -> size < newblock -> size){
						newblock -> prev_size = curr;
						newblock -> next_size = NULL;
						newblock -> prev_size -> next_size = newblock;
						break;
					}
					else if(newblock -> size < curr -> size){

						newblock -> next_size = curr;
						newblock -> prev_size = curr -> prev_size;
						curr -> prev_size -> next_size = newblock;
						newblock -> next_size -> prev_size = newblock;
						break;
					}
					curr = curr -> next_size;
					if(curr == NULL)
						break;
				} 
				
			}

		} else{
			currentNode = currentNode -> next_size;
			another_node = another_node -> next_addr;
		}
		
	}
	printf("%d\n", (int)size);
	if(size_list == NULL && address_list == NULL){
		metadata_t *block = my_sbrk(SBRK_SIZE);
		if(block == NULL){
			my_malloc_errno = OUT_OF_MEMORY;
			return NULL;
		} else{
			block -> canary = ((uintptr_t)block ^ CANARY_MAGIC_NUMBER) + 1;
			size_list = block;
			address_list = block;
			block -> size = SBRK_SIZE - size_needed;
			metadata_t *split= (metadata_t*)((uint8_t*)block + block -> size - size_needed);
			split -> size =  size_needed;
			

			split -> canary = ((uintptr_t)split ^ CANARY_MAGIC_NUMBER) + 1;
			*((unsigned long*)((uint8_t*)split + size_needed - sizeof(unsigned long))) = split -> canary;
		


			

			printf("%s\n","hi" );
			return  (uint8_t* ) split  + sizeof(metadata_t);;
			
		}
	}
	

	return NULL;
    //return currentNode + TOTAL_METADATA_SIZE
}

/* REALLOC
 * See my_malloc.h for documentation
 */
void *my_realloc(void *ptr, size_t size) {
	my_malloc_errno = NO_ERROR;
	if(ptr == NULL){
		return my_malloc(size);
	} else{
		printf("(%d)\n",(int)size );
		metadata_t *meta = (metadata_t *)ptr - 1;
		printf("%d\n", (int)meta -> size);
		long unsigned int canary = meta -> canary;
		if(canary != ((uintptr_t)meta ^ CANARY_MAGIC_NUMBER) + 1 
				|| *((unsigned long*)((uint8_t*)meta + meta -> size - sizeof(unsigned long))) != canary){
			my_malloc_errno = CANARY_CORRUPTED;
			return NULL;
		} else if( size == 0){
			my_free(ptr);
		} else{
			uint8_t *newblock = my_malloc(size);
			memcpy(newblock, ptr, size);
			my_free(ptr);
			return newblock;
		}

	}
	UNUSED_PARAMETER(ptr);
	UNUSED_PARAMETER(size);
	return (NULL);
	}

/* CALLOC
 * See my_malloc.h for documentation
 */
void *my_calloc(size_t nmemb, size_t size) {
	void* returnptr =  my_malloc(nmemb*size);
	if(returnptr == NULL)
		return NULL;
	returnptr = memset(returnptr, 0, size);
	metadata_t *meta = (metadata_t *)returnptr - 1;
	meta -> prev_addr = NULL;
	meta -> next_addr = NULL;
	meta -> prev_size = NULL;
	meta -> next_size = NULL;
	return returnptr;
	
}

/* FREE
 * See my_malloc.h for documentation
 */
void my_free(void *ptr) {		
	my_malloc_errno = NO_ERROR;
	printf("%d\n", my_malloc_errno);
	if(ptr != NULL){
		metadata_t *meta = (metadata_t *)ptr - 1;
		if(1 == 2){
			meta = NULL;
		}
		else{
			long unsigned int canary = meta -> canary;
			if(canary != ((uintptr_t)meta ^ CANARY_MAGIC_NUMBER) + 1 
				|| *((unsigned long*)((uint8_t*)meta + meta -> size - sizeof(unsigned long))) != canary){
				my_malloc_errno = CANARY_CORRUPTED;
				printf("%lu\n", canary);
				printf("%lu\n", ((uintptr_t)meta ^ CANARY_MAGIC_NUMBER) + 1);
				printf("%lu\n", *((unsigned long*)((uint8_t*)meta + meta -> size - sizeof(unsigned long))));
			}else if(address_list == NULL){
				my_malloc_errno = NO_ERROR;
				address_list = meta;
				size_list = meta;
					printf("hi\n");

			} else{
				metadata_t *another_node = address_list;
				printf("%d\n", my_malloc_errno);
				while(another_node != NULL){
					//left merge
					if((void*)((uint8_t *)another_node + another_node -> size)  == (void*)meta){
						another_node -> size += meta -> size;
						if(another_node -> prev_size == NULL)
						size_list = another_node -> next_size;
						if(another_node -> prev_size != NULL)
							another_node -> prev_size -> next_size = another_node -> next_size;
						if(another_node -> next_size != NULL)
							another_node -> next_size -> prev_size = another_node -> prev_size;

						metadata_t *curr = size_list;

						while(curr != NULL){

							if(curr -> prev_size == NULL && another_node -> size <= curr -> size ){
							
								
								another_node -> next_size = curr;
								another_node -> prev_size = NULL;
								another_node -> next_size -> prev_size = another_node;

								size_list = another_node;
								break;
							} else if(curr -> next_size == NULL && curr -> size < another_node -> size){
								another_node -> prev_size = curr;
								another_node -> next_size = NULL;
								another_node -> prev_size -> next_size = another_node;
								break;
							}
							else if(another_node -> size < curr -> size){

								another_node -> next_size = curr;
								another_node -> prev_size = curr -> prev_size;
								curr -> prev_size -> next_size = another_node;
								another_node -> next_size -> prev_size = another_node;
								break;
							}
							curr = curr -> next_size;
							if(curr == NULL)
								break;
						} 
						if(another_node -> next_addr != NULL){
							if((void*)((uint8_t *)another_node + another_node -> size)  == (void*)another_node -> next_addr){
								another_node -> size += another_node -> next_addr -> size;
								metadata_t *rodney = another_node -> next_addr;
								if(rodney-> prev_size != NULL)
									rodney-> prev_size -> next_size = rodney -> next_size;
								if(rodney -> next_size != NULL)
									rodney -> next_size -> prev_size =rodney -> prev_size;

								if(rodney -> prev_addr != NULL)
									rodney -> prev_addr -> next_addr = rodney -> next_addr;
								if(rodney -> next_addr != NULL)
									rodney -> next_addr -> prev_addr = rodney -> prev_addr;	


								if(another_node -> prev_size == NULL)
									size_list = another_node -> next_size;
								if(another_node -> prev_size != NULL)
									another_node -> prev_size -> next_size = another_node -> next_size;
								if(another_node -> next_size != NULL)
									another_node -> next_size -> prev_size = another_node -> prev_size;



								curr = size_list;

								while(curr != NULL){

									if(curr -> prev_size == NULL && another_node -> size <= curr -> size ){
									
										
										another_node -> next_size = curr;
										another_node -> prev_size = NULL;
										another_node -> next_size -> prev_size = another_node;

										size_list = another_node;
										break;
									} else if(curr -> next_size == NULL && curr -> size < another_node -> size){
										another_node -> prev_size = curr;
										another_node -> next_size = NULL;
										another_node -> prev_size -> next_size = another_node;
										break;
									}
									else if(another_node -> size < curr -> size){

										another_node -> next_size = curr;
										another_node -> prev_size = curr -> prev_size;
										curr -> prev_size -> next_size = another_node;
										another_node -> next_size -> prev_size = another_node;
										break;
									}
									curr = curr -> next_size;
									if(curr == NULL)
										break;
								} 
							}
						}
						
						goto end;
					}/*right merge*/ else if((void*)((uint8_t *)meta + meta -> size)  == (void*)another_node){
						meta -> next_addr = another_node -> next_addr;
						meta -> prev_addr = another_node -> prev_addr;
						meta -> size  += another_node -> size;

						if(meta -> prev_addr == NULL)
							address_list = meta;
						if(another_node -> prev_addr != NULL)
							another_node -> prev_addr -> next_addr = meta;
						if(another_node -> next_addr != NULL)
							another_node -> next_addr -> prev_addr = meta;
						if(another_node -> prev_size != NULL)
							another_node -> prev_size -> next_size = another_node -> next_size;
						if(another_node -> next_size != NULL)
							another_node -> next_size -> prev_size = another_node -> prev_size;


						metadata_t *curr = size_list;

						while(curr != NULL){

							if(curr -> prev_size == NULL && meta -> size <= curr -> size ){
							
								
								meta -> next_size = curr;
								meta -> prev_size = NULL;
								meta -> next_size -> prev_size = meta;

								size_list = meta;
								break;
							} else if(curr -> next_size == NULL && curr -> size < meta -> size){
								meta -> prev_size = curr;
								meta -> next_size = NULL;
								meta -> prev_size -> next_size = meta;
								break;
							}
							else if(meta -> size < curr -> size){

								meta -> next_size = curr;
								meta -> prev_size = curr -> prev_size;
								curr -> prev_size -> next_size = meta;
								meta -> next_size -> prev_size = meta;
								break;
							}
							curr = curr -> next_size;
							if(curr == NULL)
								break;
						}
						goto end;
					} 
					another_node = another_node -> next_addr;
				}
				metadata_t *curr = address_list;

				while(curr != NULL){

					if(curr -> prev_addr == NULL && meta < curr ){
					

						meta -> next_addr = curr;
						meta -> prev_addr = NULL;
						meta -> next_addr -> prev_addr = meta;

						address_list = meta;
						break;
					} else if(curr -> next_addr == NULL && curr < meta ){
						meta -> prev_addr = curr;
						meta -> next_addr = NULL;
						meta -> prev_addr -> next_addr = meta;
						break;
					}
					else if(meta < curr){

						meta -> next_addr = curr;
						meta -> prev_addr = curr -> prev_addr;
						curr -> prev_addr -> next_addr = meta;
						meta -> next_addr -> prev_addr = meta;
						break;
					}
					curr = curr -> next_addr;
					if(curr == NULL)
						break;
				} 

				curr = size_list;

				while(curr != NULL){

					if(curr -> prev_size == NULL && meta -> size <= curr -> size ){
					
						
						meta -> next_size = curr;
						meta -> prev_size = NULL;
						meta -> next_size -> prev_size = meta;

						size_list = meta;
						goto end;
					} else if(curr -> next_size == NULL && curr -> size < meta -> size){
						meta -> prev_size = curr;
						meta -> next_size = NULL;
						meta -> prev_size -> next_size = meta;
						goto end;
					}
					else if(meta -> size < curr -> size){

						meta -> next_size = curr;
						meta -> prev_size = curr -> prev_size;
						curr -> prev_size -> next_size = meta;
						meta -> next_size -> prev_size = meta;
						goto end;
					}
					curr = curr -> next_size;
					if(curr == NULL)
						goto end;
				}
			}
		}


		
		
	}
	end: 
	
	UNUSED_PARAMETER(ptr);
}
