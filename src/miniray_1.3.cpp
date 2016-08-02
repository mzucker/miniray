#include <stdio.h>
#include <math.h>
#define O operator
#define E return

typedef float f;

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
} L(!v(-1,1,2));

typedef unsigned char u;
typedef int i;

// hit query struct for successive distance queries
struct q: v {

  // *this stores point being queried
  v c; // c: closest in geometry
  f d; // d: distance from p to c

  // construct with large distance
  q(v P): v(P), d(99) {}

  // see if new point n is closer to p than current point c
  // and modify distances if so
  i operator()(v n) { 
    f e= ~(*this-n);
    if (e<d) { c=n; d=e; }
  }

};

f U(f a) { return a<0?0:a>1?1:a; }

// test distance from point h to a circular arc centered at point x,
// y, (z=0) with radius r and angle range given by a & b
i C(f x, f y, f r, f a, f b, q& h) {
  f t = atan2(h.y-y,h.x-x);
  t = t<a?a:t>b?b:t;
  h(v(x+r*cos(t),y+r*sin(t)));
}

// test distance from point h to a line segment going from (x,y) to
// (x+s,y+t) (z=0).
i S(f x,f y,f s,f t,q& h) {
  v a(x,y), d(s,t);
  f u=(h-a)%d/(d%d);
  h(a+d*U(u));
}

i P(v n, q& h) { h(h-n*(h%n+.9)); }

// test distance from point p to the text "mattz"
f D(v p, v& n) {

  // initialize our distance query
  q h(p);

  // encode "mattz" as bytecodes: see encode.cpp for details
  u d[] = "@`B@D@\x82$\x86$\x8d\xe3H`J\x81)c\x96""BN\x81-c\x9e""B1`1cq`";

  // interpret bytecodes by calling S() and C() with the appropriate
  // parameters
  for (u* c=d;*c;c+=2) {
    i o=c[0]>>5, a=c[1]>>5; // o: opcode in 0..7, a: arguments in 0..7
    f x=c[0]&31, y=c[1]&31, r=M_PI; // x,y are position in 0..31
    (o<4) ? S(x,y,a*(o&1),a*(o&2)/2,h) : 
      C(x/2,y/2,1+.5*(a/4),-r*(a&2)/2,r*(a&1),h);
  }

  P(v(0,0,1),h);
  P(v(0,1),h);

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
      f c=0, a=1, s=pow(U(n%!(L-d)),80);
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
  //v c(-2,4,25), z(c-v(8.25,2)), x=v(0,1)^z, y=x^z; 
  //fprintf(stderr, "x=%f,%f,%f\n", x.x, x.y, x.z); x=!x; 
  //fprintf(stderr, "y=%f,%f,%f\n", y.x, y.y, y.z); y=!y;
  i t=-1;
  puts("P5 600 220 255");
  while (++t<220) 
    for (i s=0;s<600;++s) {
      putchar(255*T(v(-2,4,25),
                    !(!v(5,0,2)*(s-300)*.034 - 
                      !v(-2,73)*(t-110)*.034 +
                      v(10.25,-2,-25))));
    }
}
