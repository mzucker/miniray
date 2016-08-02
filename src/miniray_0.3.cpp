#include <stdio.h>
#include <math.h>
#define op operator
#define ret return

typedef int i;
typedef float f;
typedef unsigned char u;

f M=80;
f L=1e-4;
f R=.4;

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

struct q {
  v p,c;
  f d;
  q(v P): p(P), c(P+v(2e2,0)), d(4e4) {}
  f operator()(v n) {
    f e=(p-n)%(p-n);
    if(e<d){c=n;d=e;}
    return d;
  }
};

static v A(f t) { return v(cos(t),sin(t),0); }

f a(v c,f r,f a,f b,q& h) {
  v d=v(h.p.x,h.p.y)-c;
  if (~d<L) {
    return h(c+A(a)*r);
  } else {
    d=!d;
    f t=atan2(d.y,d.x);
    return h(c+((t<a)?A(a):((t>b)?A(b):d))*r);
  }
}

f s(f x,f y,f s,f t,q& h) {
  v a(x,y);
  v d(s,t);
  f u=(h.p-a)%d/(d%d);
  return h(a+d*(u<0?0:u>1?1:u));
}

f dst(v p, v* n=0) {
  q h(p);
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
  for (u* c=(u*)d;*c;c+=2) {
    i o=c[0]>>5;
    f x=c[0]&31;
    f y=c[1]&31;
    i l=c[1]>>5;
    (o>=1&&o<=3)?s(x,y,l*(o!=2),l*(o!=1),h):
      (o==4)?a(v(x/2,y/2),1+.5*(l>>2),
               -M_PI*((l/2)&1),M_PI*(l&1),h):0;
  }
  if (n){*n=!(p-h.c);}
  return sqrt(h.d)-R;
}

v T(v o, v d) {
  v pi=o;
  f u=0;
  f l0=dst(pi);
  for (i c=0;u<M&&c<1000;++c) {
    f l1=dst(pi=o+d*(u+=.7*l0));
    if (fabs(l1)<L){
      return v(255,0,0);
    }
    l0=l1;
  }
  return v(0,0,0);
}

int main() {
  v o(10,2,30);
  i w=720;
  i h=270;
  f s=24.0/w;
  printf("P3 %d %d 255\n", w, h);
  for (i y=0; y<h; ++y) {
    for (i x=0; x<w; ++x) {
      v ip(o.x+(x-w/2)*s, o.y-(y-h/2)*s, 0);
      v d = !(ip-o);
      v c = T(o, d);
      printf("%d %d %d\n", i(c.x), i(c.y), i(c.z));
    }
  }
  return 0;
}
