#include <stdio.h>
#include "smalloc.h"

int 
main()
{
	void *p1, *p2, *p3, *p4, *p5 ;

	print_sm_containers() ;

	p1 = smalloc(2300) ; 
	printf("smalloc(2000)\n") ; 
	print_sm_containers() ;

	p2 = smalloc(2500) ; 
	printf("smalloc(2500)\n") ; 
	print_sm_containers() ;

	sfree(p1) ; 
	printf("sfree(%p)\n", p1) ; 
	print_sm_containers() ;

	p3 = smalloc(1400) ; 
	printf("smalloc(1500)\n") ; 
	print_sm_containers() ;

	p4 = smalloc(1650) ; 
	printf("smalloc(1650)\n") ; 
	print_sm_containers() ;

	p5 = smalloc(2200) ; 
	printf("smalloc(2200)\n") ; 
	print_sm_containers() ;

	print_sm_uses();
}
