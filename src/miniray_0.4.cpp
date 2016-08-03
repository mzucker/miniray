#include <stdio.h>
#include <math.h>
#define O operator
#define E return

typedef int i;
typedef float f;
typedef unsigned char u;

f M=50,   // maximum distance we care about in sphere tracing
  L=1e-4, // minimum distance tolerance
  R=.4;   // radius of letters in "mattz"

// vector struct
struct v {
  f x,y,z;
  v() {}
  v (f a, f b, f c=0): x(a),y(b),z(c) {}
  v operator*(f s) { return v(x*s,y*s,z*s); }
  f operator%(v r) { return x*r.x+y*r.y+z*r.z; }
  v operator+(v r) { return v(x+r.x,y+r.y,z+r.z); }
  v operator-(v r) { return *this+r*-1; }
  v operator!() { return *this*(1/~(*this)); }
  f operator~() { return sqrt(*this%*this); }
};

// hit query struct for successive distance queries
struct q {

  v p, c; // p: world point and c: closest in geometry
  f d; // d: squared distance from p to c

  // construct with large distance
  q(v P): p(P), c(P+v(2e2,0)), d(4e4) {} 

  // see if new point n is closer to p than current point c
  // and modify distances if so
  void operator()(v n) { 
    f e= (p-n)%(p-n);
    if (e<d) { c=n; d=e; }
  }

};

// construct a unit vector with given angle
v A(f t) { return v(cos(t),sin(t)); }

// test distance from point h.p to a circular arc centered at point x,
// y, (z=0) with radius r and angle range given by a & b
i C(f x, f y, f r, f a, f b, q& h) {
  v c(x,y); // center of circle
  v d=v(h.p.x,h.p.y)-c; // vector from projection of point p onto z=0
                        // plane to center of circle
  if (~d<L) { // degenerate because we are in the middle of the circle
    h(c+A(a)*r); // just return some point on the ring.
  } else { // general case
    d=!d; // normalize direction vector
    f t=atan2(d.y,d.x); // find angle
    h(c+((t<a)?A(a):((t>b)?A(b):d))*r); // return point on circle
  }
}

// test distance from point h.p to a line segment going from (x,y) to
// (x+s,y+t) (z=0).

i S(f x,f y,f s,f t,q& h) {
  v a(x,y), d(s,t);
  f u=(h.p-a)%d/(d%d);
  h(a+d*(u<0?0:u>1?1:u));
}

// test distance from point p to the text "mattz"
f D(v p, v* n=0) {
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
  // interpret bytecodes
  for (u* c=(u*)d;*c;c+=2) {
    i o=c[0]>>5, l=c[1]>>5;
    f x=c[0]&31, y=c[1]&31, r=M_PI;
    (o>=1&&o<=3)?S(x,y,l*(o&1),l*(o&2)/2,h):
      (o&4)?C(x/2,y/2,1+.5*(l/4),
              -r*((l&2)/2),r*(l&1),h):0;
  }
  // if we were asked for a normal, spit it out
  if (n){*n=!(p-h.c);}
  // return the distance
  return sqrt(h.d)-R;
}

v T(v o, v d) {
  v p=o;
  f u=0, l=D(p);
  while (u<M) {
    l=D(p=o+d*(u+=.9*l));
    if (fabs(l)<L){
      return v(255,0,0);
    }
  }
  return v(0,0,0);
}

int main() {
  v o(10,2,30); // camera
  i w=720, h=270, x, y=-1;
  f s=24.0/w; // disp on pixel plane
  printf("P6 %d %d 255\n", w, h);
  while (++y<h) {
    for (x=0;x<w;++x) {
      v ip(o.x+(x-w/2)*s, o.y-(y-h/2)*s),
        d = !(ip-o),
        c = T(o, d);
      printf("%c%c%c", u(c.x), u(c.y), u(c.z));
    }
  }
}
