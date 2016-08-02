#include <stdio.h>
#include <math.h>
#define S for
#define E return

typedef float f;

f U(f a) { return a<0?0:a>1?1:a; }

f H=.5, Y=.66, I, y=-111, T=-1, l=0, q, w, s, t, r;

typedef struct { f x,y,z; } v;

f O(v a, v b) { return q=a.z*b.z+b.x*a.x+a.y*b.y; }

v L, W={1,1,1}, Z={0,0,0}, P, C, M, N, K, p;

v G(f t, f a, f c) { K.x=t; K.z=c; K.y=a; return K; }

v A(v a, v b, f c) { return G(a.x+b.x*c, a.y+c*b.y, b.z*c+a.z); }

char* J = "LJFFF%7544\0#( #@@DA#(.@@%(0CAaIqDCI$IDEH%P@T@qL%PEaIpBJCAI%KBPBEP%CBPEaIqBAI%CAaIqBqDAI%U@PE%AAaIqBcDAI%ACaIaCqDCI%(aHCcIpBBH%E@aIqBAI%AAaIqB%AAaIqBEH%AAPBaIqB%PCDHxL%H@hIcBBI%E@qJBH#C@@D%aIBI@D%E@QB2P#E@'C@qJBHqJBH%C@qJBH%AAaIqBAI%C@cJ%cJCH%C@qJ%aIqB1I%PCDI`I%BAaICH%KH+@'JH+@KP*@%S@3P%H@ABhIaBBI%P@S@PC",

  B[999], *b=B, *j, *e, X=40, o, a, c, m, z=5;

f Q() {
  return A(P,M,T), O(K,K)<I ? C=M,I=q:0;
}

f D(v p) {

  
  S (r=l,w=0,I=99,P=p,b=B;(o=*b++);)

    if (o<X)
      
      s = r += w, w = o-34;
      
    else {
      
      if (m=o/8&3, a=o&7, o=o/32&3, s+=*b/8&3, t=*b++&7, M=G(s,t,0), o<3) 

        N=G((o+o-3)*m+1e-4,a,0), M=A(M,N,U(O(A(P,M,T),N)/O(N,N)));

      else {

        f i=M_PI*H, j=atan2(P.y-t, P.x-s)/i, l=m-2, d=a+1, u=l+d;
        j = i*(j>u+H*d ? l : j > u ? u : j<l-H*d ? u : j<l ? l : j);

        M.x += cos(j);
        M.y += sin(j);

      } 

      Q();

    }

  M=P; M.y=-.9;
  o=P.x/8+8;
  a=P.z/8+8;

  M = Q() ? (o^a)&1 ? G(Y,0,0) : W : G(Y,Y,1);

  N=A(P,C,T);

  return sqrt(I)-.45;

}

v _(v a) { return A(Z, a, pow(O(a,a),-H)); }

v R(v o, v d, f r) {

  f u=0, l=0, i=0;

  S (;l=D(A(o,d,u+=l)),u<97;)
    if (l<1e-3) {
      
      v p=M, n=_(N);
          
      S (o=A(o,d,u), u=1; ++i<6; u -= U(i/3-D(A(o,n,i/3)))/pow(2,i));

      p = A(Z,p,(U(O(n,L))*H*Y+Y)*u);

      p = r?A(A(Z,p,Y),R(A(o,n,.1),A(d,n,-2*O(d,n)),r-1),H*Y):p;

      u=pow(U(O(n,_(A(L,d,T)))),X);

      return A(A(Z,p,1-u),W,u);

    }

  q=d.z;
  return G(q*q,q*q,1);

}

int main(int x, char** k) {

  S (e = x>1 ? z=0, 1[k] : J; *e; ++e)
    S (o=a=0, j=J+11; (c=*j)&&!(o&&*j<X); ++j) 
      o ? (*b++=c, *b++=*++j) : 
        ( c<X ? (w=c,++a) : 
          c==X ? (a=*++j) : a,
          a == ((*e|32)^z) ? l-=34-(*b++=w), o=1 : l);

  S (z=x<3, *b = 0, l = 10-l*H,
     L=_(G(T,1,2)), puts("P6 600 220 255"); ++y<110;) 

    S (x=-301;p=Z,++x<300;putchar(p.x),putchar(p.y),putchar(p.z))
      
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
