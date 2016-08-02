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
  v operator-(v r) { return v(x-r.x,y-r.y,z-r.z); }
  v operator!() { return *this*(1/~(*this)); }
  f operator~() { return sqrt(+(*this)); }
  f operator+() { return *this%*this; }
};

struct q {
  v p,pc;
  f d;
  q(v P): p(P), pc(P+v(2e2,0)), d(4e4) {}
  f operator()(v pn) {
    f dn=+(p-pn);
    if(dn<d){pc=pn;d=dn;}
    return d;
  }
};

v fa(f t) { return v(cos(t),sin(t),0); }
f max(f a, f b) { return a>b?a:b; }
f c01(f a) { return a>1?1:max(a,0); }
v le(v a, v b, f u) { return a*(1-u)+b*u; }
v c01(v x) { return v(c01(x.x),c01(x.y),c01(x.z)); }
i fi(f c) { return c01(c)*255; }

v black(0,0,0);
v white(1,1,1);

f a(v c,f r,f a,f b,q& h) {
  v d=v(h.p.x,h.p.y)-c;
  if (~d<L) {
    return h(c+fa(a)*r);
  } else {
    d=!d;
    f t=atan2(d.y,d.x);
    return h(c+((t<a)?fa(a):((t>b)?fa(b):d))*r);
  }
}

f s(f x,f y,f dx,f dy,q& h) {
  v a(x,y);
  v d(dx,dy);
  f l=+d;
  f u=c01((h.p-a)%d/l);
  return h(a+d*u);
}

f dst(v p, v* nn=0) {
  q h(p);
  i d[] = {
    0x40426040,0x24824044,0xe38d2486,0x814a6048,
    0x42966329,0x632d814e,0x6031429e,0x60716331,
    0
  };
  for (u* c=(u*)d;*c;c+=2) {
    i o=c[0]>>5;
    f x=c[0]&31;
    f y=c[1]&31;
    i l=c[1]>>5;
    (o>=1&&o<=3)?s(x,y,l*(o!=2),l*(o!=1),h):
      (o==4)?a(v(x/2,y/2),1+.5*(l>>2),
               -M_PI*((l>>1)&1),M_PI*(l&1),h):0;
  }
  if (nn){*nn=!(p-h.pc);}
  return sqrt(h.d)-R;
}

f T(v o, v d, v& pi, v& n) {
  pi=o;
  f u=0;
  f l0=dst(pi);
  for (i c=0;u<M&&c<1000;++c) {
    f l1=dst(pi=o+d*(u+=.7*l0));
    if (fabs(l1)<L){dst(pi,&n);return u;}
    l0=l1;
  }
  return M;
}

v light(v c, v n, v l, v d) {
  f nl = max(n%l,0);
  v h = !(l-d);
  f nh = max(n%h,0);
  return c01(le(c*.25, c, nl) + white*pow(nh,120));
}

int main(int argc, char** argv) {
  v o(10,2,30);
  i nx=720;
  i ny=270;
  f dx=24.0/nx;
  v pi,n;
  v l = !v(-1,2,1);
  printf("P3 %d %d 255\n", nx, ny);
  for (i y=0; y<ny; ++y) {
    for (i x=0; x<nx; ++x) {
      v ip(o.x+(x-nx/2)*dx, o.y-(y-ny/2)*dx, 0);
      v d = !(ip-o);
      f u = T(o, d, pi, n);
      v c = (u < M) ? light(v(1,0,0),n,l,d) : v(0,0,0);
      printf("%d %d %d\n", fi(c.x), fi(c.y), fi(c.z));
    }
  }
  return 0;
}
