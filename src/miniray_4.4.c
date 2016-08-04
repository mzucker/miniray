#include <stdio.h>
#include <math.h>
#define S for
#define E return

typedef float f;

f U(f a) { return a<0?0:a>1?1:a; }

f H=.5, Y=.66, I, T=-1, l=0, q, w, u, i;

typedef struct { f x,y,z; } v;

f O(v a, v b) { return q=a.z*b.z+b.x*a.x+a.y*b.y; }

v L, W={1,1,1}, Z={0,0,0}, B[99], P, C, M, N, K, p;

int b=0, r, y=-111, (*m)(int) = putchar, o, a, c, t;

v G(f t, f a, f c) { K.x=t; K.z=c; K.y=a; return K; }

v A(v a, v b, f c) { return G(a.x+b.x*c, a.y+c*b.y, b.z*c+a.z); }

char* J = "LJFFF%7544\0#( #@@DA#(.@@%(0CAaIqDCI$IDEH%P@T@qL%PEaIpBJCAI%KBPBEP%CBPEaIqBAI%CAaIqBqDAI%U@PE%AAaIqBcDAI%ACaIaCqDCI%(aHCcIpBBH%E@aIqBAI%AAaIqB%AAaIqBEH%AAPBaIqB%PCDHxL%H@hIcBBI%E@qJBH#C@@D%aIBI@D%E@QB2P#E@'C@qJBHqJBH%C@qJBH%AAaIqBAI%C@cJ%cJCH%C@qJ%aIqB1I%PCDI`I%BAaICH%KH+@'JH+@KP*@%S@3P%H@ABhIaBBI%P@S@PC",

  *j, *e, X=40, z=5;

f Q() {
  return A(P,M,T), O(K,K)<I ? C=M,I=q:0;
}

f D(v p) {

  S(I=99, P=p, b=0; 
    M = B[b++], M.x+=l, N = B[b], b<r; 
    ++b, Q()) 
    
    M = N.z
    ?
    q = M_PI*H, w=atan2(P.y-M.y, P.x-M.x)/q, o=N.x-2, a=N.y+1, t=o+a,
    w = q*(w>t+H*a ? o : w > t ? t : w<o-H*a ? t : w<o ? o : w),
    A(M, G(cos(w), sin(w), 0), 1)
    :
    A(M,N,U(O(A(P,M,T),N)/O(N,N)));


  M=P; M.y=-.9;
  o=P.x/8+8;
  a=P.z/8+8;

  M = Q() ? (o^a)&1 ? G(Y,0,0) : W : G(Y,Y,1);

  N=A(P,C,T);

  return sqrt(I)-.45;

}

v _(v a) { return A(Z, a, pow(O(a,a),-H)); }

v R(v o, v d, f r) {


  S (u=i=0;i=D(A(o,d,u+=i)),u<97; )
    if (i<1e-3) {
      
      v p=M, n=_(N);
          
      S (o=A(o,d,u), u=1, i=0; ++i<6; u -= U(i/3-D(A(o,n,i/3)))/pow(2,i));

      p = A(Z,p,(U(O(n,L))*H*Y+Y)*u);

      p = r?A(A(Z,p,Y),R(A(o,n,.1),A(d,n,-2*O(d,n)),r-1),1-Y):p;

      u=pow(U(O(n,_(A(L,d,T)))),X);

      return A(A(Z,p,1-u),W,u);
      

    }

  q=d.z;
  return G(q*q,q*q,1);

}

int main(int x, char** k) {
  
  S (e = x>1 ? z=0, 1[k] : J; *e; ++e) 
    S (o=a=0, j=J+11; (c=*j)&&!(o&&c<X&&(q=l+=w)); ++j) 
    o ? o=*j++/32&3, 
    B[b++] = G(q+=*j/8&3,*j&7,0),
    B[b++] = G((c/8&3)*(o<2?T:1), (c&7)+1e-4*(o<3), o>2), 1  :
    (o = (a = c<X ? w=c-34, a+1 : c>X ? a : *++j) == ((*e|32)^z)) ;

  S (r=b, z=x<3, l = 10-l*H,
     L=_(G(T,1,2)), puts("P6 600 220 255"); ++y<110;) 

    S (x=-301;p=Z,++x<300;m(p.x),m(p.y),m(p.z))
      
    S (c=0; c<z*3+1; ++c) 
    
    p = A(p, R( G(-4.5,5,30),
                _( A( A( A(Z, _(G(5,0,2)), x+c/2*H ), 
                         _(G(2,-73,0)),y+H*(c&1)),
                      G(30.75,-6,-75),
                      10) ),
                z+z ), 
          z?63:255 );

  return 0;

}
