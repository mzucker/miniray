#include <stdio.h>
#include <math.h>
#include <iostream> 
#define O operator
#define E return

typedef int i;
typedef float f;
typedef unsigned char u;

// vector struct
struct v {
  f x,y,z;
  v (f a=0, f b=0, f c=0): x(a),y(b),z(c) {}
  v operator*(f s) { return v(x*s,y*s,z*s); }
  f operator%(v r) { return x*r.x+y*r.y+z*r.z; }
  v operator+(v r) { return v(x+r.x,y+r.y,z+r.z); }
  v operator-(v r) { return *this+r*-1; }
  v operator!() { return *this*(1/~(*this)); }
  f operator~() { return sqrt(*this%*this); }
  //v operator^(v r) { return v(y*r.z-z*r.y,x*r.z-z*r.x,x*r.y-y*r.x); }
} L(!v(-1,1,2)), W(1,1,1), B;

// hit query struct for successive distance queries
struct q {

  v p, c; // p: world point and c: closest in geometry
  f d; // d: distance from p to c

  // construct with large distance
  q(v P): p(P), d(9e9) {}

  // see if new point n is closer to p than current point c
  // and modify distances if so
  i operator()(v n) { 
    f e= ~(p-n);
    if (e<d) { c=n; d=e; }
  }

};

f U(f a) { return a<0?0:a>1?1:a; }

// test distance from point h.p to a circular arc centered at point x,
// y, (z=0) with radius r and angle range given by a & b
i C(f x, f y, f r, f a, f b, q& h) {
  f t = atan2(h.p.y-y,h.p.x-x);
  t = t<a?a:t>b?b:t;
  h(v(x+r*cos(t),y+r*sin(t)));
}

// test distance from point h.p to a line segment going from (x,y) to
// (x+s,y+t) (z=0).
i S(f x,f y,f s,f t,q& h) {
  v a(x,y), d(s,t);
  f u=(h.p-a)%d/(d%d);
  h(a+d*U(u));
}

i P(v n, q& h) { h(h.p-n*(h.p%n+.9)); }

// test distance from point p to the text "mattz"
f D(v p, v& n) {

  // initialize our distance query
  q h(p);

  // encode "mattz" as bytecodes
  i d[] = {
    1078091840,
    612515908,
    -477289338,
    -2125832120,
    1117152041,
    1663926606,
    1613841054,
    1618043697,
    0
  };

  // interpret bytecodes by calling S() and C() with the appropriate
  // parameters
  for (u* c=(u*)d;*c;c+=2) {
    i o=c[0]>>5, a=c[1]>>5; // o: opcode in 0..7, a: arguments in 0..7
    f x=c[0]&31, y=c[1]&31, r=M_PI; // x,y are position in 0..31
    (o<4) ? S(x,y,a*(o&1),a*(o&2)/2,h) : 
      (o&4) ? C(x/2,y/2,1+.5*(a/4),-r*(a&2)/2,r*(a&1),h) : 0;
   
  }

  P(v(0,0,1),h);
  P(v(0,1,0),h);

  // if we were asked for a normal, spit it out
  n=!(p-h.c);

  // return the distance
  return h.d-.45;

}

// do our raytracing from ray origin o in direction d
f T(v o, v d) {

  //////////////////////////////////////////////////
  // first do the sphere tracing for letters
  
  f u=0, l=0; // u is dist along ray, l is dist to obj
  v n;

  while (u<50) 
    if (fabs(l=D(o+d*(u+=.9*l),n))<1e-4){
      // do ambient occlusion pass
      f c=0, a=1, s=pow(U(n%!(L-d)),40);
      o=o+d*u;
      while (++c<6) { 
        a -= .8*(.2*c-D(o+n*.2*c,d))/pow(2,c);
      }
      return ((.3*U(n%L)+.65)*(1-s)+s)*a;
    }
  
  //////////////////////////////////////////////////

  return 0;

}


i main() {
  v c(-2,4,25), z(c-v(8.25,2)), x=!v(5,0,2), y=!v(-2,73,.5);
  //v c(-2,4,25), z(c-v(8.25,2)), x=v(0,1)^z, y=x^z; 
  //fprintf(stderr, "x=%f,%f,%f\n", x.x, x.y, x.z); x=!x; 
  //fprintf(stderr, "y=%f,%f,%f\n", y.x, y.y, y.z); y=!y;
  i w=600, h=220, s, t=-1;
  //i w=300, h=110, s, t=-1;
  f p=20.0/w;
  printf("P5 %d %d 255\n", w, h);
  while (++t<h) 
    for (s=0;s<w;++s) {
      v d = !(x*(s-w/2)*p - y*(t-h/2)*p - z);
      printf("%c", u(T(c,d)*255));
    }
}
