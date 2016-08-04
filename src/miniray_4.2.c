#include <stdio.h>
#include <math.h>
#define E return

typedef float f;
f H=.5, Z=.33, Y=.66, I, y=-111, t=0, q, w, u, m, r;
f U(f a) { return a<0?0:a>1?1:a; }

typedef struct { f c,a,t; } v;

v L, W={1,1,1}, F={.7,.7,1}, P, C, M, N, k, d, K;

v G(f t, f a, f c) { K.c=t; K.a=a; K.t=c; return K; }
v A(v a, v b, f c) { return G(a.c+b.c*c, a.a+b.a*c, a.t+b.t*c); }
f O(v a, v b) { return a.t*b.t+a.c*b.c+a.a*b.a; }
f Q(v c, v m) {
  K = A(P,c,-1);
  q = O(K,K);
  return q<I ? C=c,M=m,I=q:I;
}

char* T = "%aHCcIhBBH%bE@aIiBAI%dAAaIiBEH%eAAPBaIiB%fPCDHlL%gH@dIcBBI%hE@iJBH%jaIBI@D%kE@QB2P#lE@'mC@iJBHiJBH%nC@iJBH%pC@cJ%qcJCH%rC@iJ%saIiB1I%tPCDI`I%uBAaICH%vKH+@'wJH+@KP*@%xS@3P%yH@ABdIaBBI%zP@S@PC#.@@#!@@DA%3PEaIhBJCAI%4KBPBEP%5CBPEaIiBAI%6CAaIiBiDAI%7U@PE%8AAaIiBcDAI%9ACaIaCiDCI#iC@@D%oAAaIiBAI%cAAaIiB%cAAaIiB%cAAaIiB# %2P@T@iL%0CAaIiDCI$1IDEH$1IDEH",

  J[999], *B, *s, *l=J, o, a, e, z;

f D(v p) {


  r=t;
  w=0;

  I=99;
  P=p;

  for (s=B;*s;++s) 

    if (*s<42) 
      
      u = r += w, w = *s++-34;
  
    else {

      o=*s/32&3; a=*s++&31;
      u+=*s/8&3; m=*s&7;
      k=G(u,m,0); d=G((2*o-3)*a/8,(a&7)+1e-4,0);

      if (o<3) 

        Q(A(k,d,U(O(A(p,k,-1),d)/O(d,d))),F);

      else {

        f P=M_PI*H, t=atan2(p.a-m, p.c-u)/P, L=(a/4&3)-2, X=(a&3)+1, U=L+X;
        t = t>U+H*X ? L : t > U ? U : t<L-H*X ? U : t<L ? L : t;

        k.c += cos(t*P);
        k.a += sin(t*P);

        Q(k,F);
        
      } 

    }


  N=G(0,1,0); Q(A(p,N,-O(p,N)-.9),W);

  M = M.c==1 & ((int)(p.c+64)/8^(int)(p.t+64)/8)&1 ?
    G(Y,0,0) : M;

  N=A(P,C,-1);

  return sqrt(I)-.45;

}

v S(v a, f s) { return G(a.c*s, a.a*s, a.t*s); }
v _(v a) { return S(a, 1/sqrt(O(a,a))); }

v R(v o, v d, f z) {

  f u=0, l=0, i=0, a=1, k=d.t*d.t;
  v p, n;

  while (u<97)
    if ((l=D(p = A(o,d,u+=l)))*l<.001) {
      
      p=M;
      n=_(N);
          
      o=A(o,d,u);

      while (++i<6)  

        a -= U(i/5-D(A(o,n,i/5)))/pow(2,i);

      p = S(p,(U(O(n,L))*Z+Y)*a);
      
      p = z?A(S(p,Y),R(A(o,n,.1),A(d,n,-2*O(d,n)),z-1),Z):p;

      u=pow(U(O(n,_(A(L,d,-1)))),40);

      return A(S(p,1-u),W,u);

    }
  
  return G(k,k,1);

}

int main(int x, char** k) {

  z=x<3;

  if (x<2) 
    B = T+274, t=-2;
  else {
    for (s=k[1];*s;++s) 
      for (o=0, B=T; *B&&!(o&*B<42);B+=2) 
        (o|*B<42&B[1]==*s) ?
          (t += !o*(*B-34),
           *l++ = B[0],
           *l++ = B[1],
           o = 1) : t;
    *l=0;
    B = J;
    t=10-t/2;
  }

  L=_(G(-1,1,2));
  
  puts("P6 600 220 255");
  

  while (++y<110) 

    for (x=-300;x<300;++x) {
      
      v p = {0,0,0};

      for (e=0; e<z*3+1; ++e)
          p = A(p, R( G(-4.5,5,30),
                      _( A( G(10.25,-2,-25) ,
                            A( S( _(G(5,0,2)), x+e/2*H ), 
                               _(G(-2,73,0)),-y-H*(e&1)), .034) ),
                      2*z ), 
                z?63:255 );

      putchar(p.c);
      putchar(p.a);
      putchar(p.t);


    }

  return 0;

}
