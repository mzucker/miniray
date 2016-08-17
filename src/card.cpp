#include <stdio.h> // card > mattz.ppm
#include <math.h> 
#define O operator
#define E return // web: goo.gl/JM9c2P
typedef float f;f H=.5,Z=.33,Y=Z+Z,S=-
1,I,y=-111;struct v{f x,y,z;v(f a=0,f
b=0,f c=0):x(a),y(b),z(c){}v O*(f s){E
v(x*s,y*s,z*s);}f O%(v r){E x*r.x+y*r.
y+z*r.z;}v O!(){v&t=*this;E t*pow(t%t,
-H);}v O+(v r){E v(x+r.x,y+r.y,z+r.z);
}}L=!v(S,1,2),W(1,1,1),P,C,M,N,A;f U(f
a){E a<0?0:a>1?1:a;}f Q(v c){M=P+c*S;f
d=M%M;E d<I?C=c,I=d:0;}f D(v p){f x=0;
I=99;P=p;for(const char*b="BCJB@bJBHb\
JCE[FLL_A[FLMCA[CCTT`T";*b;++b){x+=*b/
4&15;int o=*b&3,a=*++b&7,y=*b/8&7;v k(
x,y),d(o%2*a,o/2*a);if(o)Q(k+d*U((p+k*
S)%d/(d%d)));else{f t=atan2(p.y-y*H,p.
x-x*H),l=a/4%2*-M_PI,u=a/2%2*M_PI;t=t<
l?l:t>u?u:t;Q(k*H+v(cos(t),sin(t))*(a%
2*H+1));}}M=Q(v(p.x,-.9,p.z))?(int((p.
x+64)/8)^int((p.z+64)/8))&1?Y:W:v(Y,Y,
1);N=P+C*S;E sqrt(I)-.45;}v R(v o,v d,
f z){for(f u=0,l=1,i=0,a=1;u<97;u+=l=D
(o+d*u))if(l<.01){v p=M,n=!N;for(o=o+d
*u;++i<6;)a-=U(i/3-D(o+n*i*.3))/pow(2,
i);p=p*(U(n%L)*Z+Y)*a;if(z)p=p*Y+R(o+n
*.1,d+n*-2*(d%n),z-1)*Z;u=pow(U(n%!(L+
d*S)),40);E p+p*-u+W*u;}z=d.z*d.z;E v(
z,z,1);}int main(){for(puts("P6 600 2\
20 255");++y<110;)for(f x=-301;++x<300
;putchar(A.z))putchar((A=R(v(-2,4,25),
!((!v(5,0,2)*x+!v(-2,73)*-y)+v(301,-59
,-735)),2)*255).x),putchar(A.y);}
