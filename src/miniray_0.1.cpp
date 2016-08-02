#include <stdio.h>
#include <math.h>
#include <iostream>

typedef int i;
typedef float f;
typedef unsigned char u;

f M = 100;
f L = 1e-4;
f R = 0.4;

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

struct hit {
  v p, pc;
  f d2;
  hit(v P): p(P), pc(P+v(2*M*M,0)), d2(2*M*M) {}
  f operator()(v pn) {
    f d2n = +(p-pn);
    if (d2n < d2) { pc=pn; d2=d2n; }
    return d2;
  }
};

v fa(f t) { return v(cos(t),sin(t),0); }

f min(f a, f b) { return a<b?a:b; }
f max(f a, f b) { return a>b?a:b; }
f c01(f a) { return min(max(a,0),1); }
v lerp(v a, v b, f u) { return a*(1-u)+b*u; }
v c01(v x) { return v(c01(x.x),c01(x.y),c01(x.z)); }
i fi(f c) { return c01(c)*255; }

v black(0,0,0);
v white(1,1,1);

template <class D> float T(v o, v d, D& k, v& pi, v& n) {
  pi=o;
  f u=0;
  f l0=k(pi, 0);
  for (i c=0;u<M&&c<1000;++c) {
    f l1=k(pi=o+d*(u+=0.7*l0), 0);
    if (fabs(l1)<L){ k(pi, &n); return u; }
    l0=l1;
  }
  return M;
}

f arc(v c,f r,f a,f b,hit& h) {
  v q=v(h.p.x,h.p.y)-c;
  if (~q<L) { 
    return h(c+fa(a)*r);
  } else {
    q=!q;
    f t = atan2(q.y, q.x);
    return h(c + ((t < a) ? fa(a) : ((t > b) ? fa(b) : q)) * r);
  }
}

f seg(v a,v b,hit& h) {
  f l = +(b-a);
  f u = l?-((a-h.p)%(b-a))/l:0;
  return h((u<0)?a:((u>1)?b:lerp(a,b,u)));
}

f seg(f x,f y,f dx,f dy,hit& h) {
  return seg(v(x,y),v(x,y)+v(dx,dy),h);
}

f dst(v p, v* nn) {

  hit h(p);

  i data[] = {
    0x40426040,
    0x24824044,
    0xe38d2486,
    0x814a6048,
    0x42966329,
    0x632d814e,
    0x6031429e,
    0x60716331,
    0
  };

  for (u* c=(u*)data; *c; c+=2) {
    i o = c[0]>>5;
    f x = c[0]&31;
    f y = c[1]&31;
    i l = c[1]>>5;
    (o>=1&&o<=3)?seg(x,y,l*(o!=2),l*(o!=1),h):
      (o==4)?arc(v(x/2,y/2),1+0.5*(l>>2),-M_PI*((l>>1)&1),M_PI*(l&1),h):0;
  }

  if (nn) { *nn = !(p-h.pc); }
  return sqrt(h.d2)-R;

}

f dst0(v p) {
  return dst(p, 0);
}

v light(v c, v n, v l, v d) {
  f nl = max(n%l,0);
  v h = !(l-d);
  f nh = max(n%h,0);
  return c01(lerp(c*0.25, c, nl) + white*pow(nh,120));
}

int main(int argc, char** argv) {
  v o(10,2,30);
  i nx=480;
  i ny=180;
  f dx=24.0/nx;
  v pi,n;
  v l = !v(-1,2,1);
  printf("P3 %d %d 255\n", nx, ny);
  for (i y=0; y<ny; ++y) {
    for (i x=0; x<nx; ++x) {
      v ip(o.x+(x-nx/2)*dx, o.y-(y-ny/2)*dx, 0);
      v d = !(ip-o);
      f u = T(o, d, dst, pi, n);
      v c = (u < M) ? light(v(1,0,0),n,l,d) : v(0,0,0);
      //if (u < M) { std::cerr << "."; }
      //f u = obj(ip);
      //v c = (u < L) ? v(1,0,0) : v(0,0,0);
      printf("%d %d %d\n", fi(c.x), fi(c.y), fi(c.z));
    }
  }

  return 0;

}

