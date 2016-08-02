//////////////////////////////////////////////////////////////////////
// This is the "verbose" version of an obfuscated business card 
// sized raytracer. 
//
// Compile this with full optimization and pipe the output to a file
// named "mattz.pgm"

#include <stdio.h>
#include <math.h>

// Start by saving some space:
#define O operator
#define E return

typedef float f;

//////////////////////////////////////////////////////////////////////
// A basic vector class. Note all vector operations are overloaded
// single character operators -- no "named" methods. Thanks, Bjarne!

struct v {

  // Just hold x,y,z.
  f x, y, z;

  // Construct with 1, 2, or 3 elements
  v (f a=0, f b=0, f c=0): x(a), y(b), z(c) {}

  // Multiplication by scalar.
  v operator*(f s) { return v(x*s, y*s, z*s); }

  // Dot product
  f operator%(v r) { return x*r.x + y*r.y + z*r.z; }

  // Return a normalized version of this vector.
  v operator!() { return *this*(1/sqrt(*this%*this)); }

  // Vector addition
  v operator+(v r) { return v(x+r.x, y+r.y, z+r.z); }

  // Vector subtraction (defined in terms of multiplication by -1 and
  // addition to save space).
  v operator-(v r) { return *this+r*-1; }


} 

// The vector L holds the light direction in world space. Defining it
// right after the class declaration saves us a character or two later
// on.
L(!v(-1,1,2));

//////////////////////////////////////////////////////////////////////
// Clamp any float to the [0,1] unit interval.

f U(f a) { return a<0?0:a>1?1:a; }

//////////////////////////////////////////////////////////////////////
// A helper function for distance queries. The scene is broken up into
// a number of primitives. For some point p, we cycle through all the
// primitives to see which one is closest to p, and this function is
// called once per primitive. Variables in this function:
//
//  n: the nearest point on the current primitive to p
//  j: the "color" of the current primitive
// 
//  c: the closest point to p on any primitive found so far
//  d: the distance between p and c
//  m: the "color" of the closest primitive identified so far
// 
// The implementation is dead simple: if the distance between p and n
// is less than d, we simply update c, d, and m.

f Q(v p, v n, v& c, f& d, f& m, f j=1) {

  f e = sqrt((p-n)%(p-n));

  if (e<d) { c=n; d=e; m=j; }

  // Return values are overrated.

}

//////////////////////////////////////////////////////////////////////
// Test the distance from the point p to the scene, and store the
// scene normal (normalized difference between p and closest point in
// the scene) in n, and the "color" of the closest primitive in m.

f D(v p, v& n, f& m) {

  // C, D, m are the variables associated with the distance query
  // function above.
  v C;
  f D=99;
  m=1;

  // Encode the letters "mattz", see below for explanation:
  unsigned char *c, 
    d[] = "@B\xe3\x8d`H\x81Jc)B\x96\x81Nc-B\x9e`1c1`q@D$\x82$\x86`@";

  // Interpret bytecodes by iterating over d in 2-byte chunks.
  for (c=d;*c;c+=2) {

    // Extract o, a, x, y from the current two bytes:
    //
    //   o is a 3-bit opcode that stores the current geometry type
    //   (line segment or arc)
    //
    //   a is a 3-bit argument whose interpretation depends on whether
    //   we are a line segment or arc
    //
    //   x and y are 5-bit scene coordinates.
    //
    int o=c[1]>>5, a=*c>>5; 
    f x=c[1]&31, y=*c&31; 

    if (o>3) { // High bit is set

      // The current primitive is an arc centered around
      // (x,y) with starting angle l, ending angle u,
      // and radius r, where
      //
      //   l = -M_PI if the middle bit of a is set, otherwise 0.
      //   
      //   u = M_PI if the low bit of a is set, otherwise 0.
      //
      //   r = 1.5 if the high bit of a is set, otherwise 1.
      //
      // Also compute t, the angle of the point p with respect to the
      // arc's center.
      //
      f r=.5*(a/4)+1, t=atan2(p.y-y/2,p.x-x/2), 
        P=M_PI, l=-P*(a&2)/2, u=P*(a&1);

      // Clamp t to the range given.
      t = t<l?l:t>u?u:t;

      // Compute the point along the arc closest to p (just given by
      // the angle t and radius r), and feed it to the distance query.
      Q(p,v(r*cos(t)+x/2,r*sin(t)+y/2),C,D,m,.7);

    } else { // High bit not set

      // The current primitive is a line segment extending
      // from (x,y) to (x+dx, y+dy), where:
      //
      //   dx = a if the low bit of o is set, otherwise zero.
      //
      //   dy = a if the middle bit of o is set, otherwise zero.
      //
      // Here, k stores the point (x,y), d stores the vector (dx,dy)
      //
      v k(x,y), d(a*(o&1),a*(o&2)/2);

      // Compute the closest point along the line segment and
      // feed it to the distance query. 
      Q(p,k+d*U((p-k)%d/(d%d)),C,D,m,.7);

    } // Done with current primitive

  } // Looping though primitives

  // Find the projections of point p onto the wall and floor planes,
  // and feed them to the distance query.
  n=v(0,1); Q(p,p-n*(p%n+.9),C,D,m);
  n=v(0,0,1); Q(p,p-n*(p%n+.9),C,D,m);

  // Compute the normal n.
  n=!(p-C);

  // Return the distance -- we subtract off a radius of .45 to give
  // everything some dimension (otherwise all primitives would be
  // infinitely thin and we would only see the planes).
  return D-.45;

}

//////////////////////////////////////////////////////////////////////
// Trace a ray from origin o along direction d, and return the scene
// brightness at the intersection. Instead of analytic ray tracing,
// this is actually "sphere tracing" by using the distance to closest
// point in scene to lower-bound how far we can step along the ray.
// Returns the scene color at the point of intersection.

v T(v o, v d) {

  // Some local variables:
  //   u: distance along the ray
  //   l: distance to closest object at u
  //   m: primitive "color"
  //   c: a counter
  //   a: ambient occlusion parameter
  //
  f u=0, l=0, m=0, c=0, a=1; 

  // n holds the normal returned by the distance query above
  v n;

  // March along the ray:
  while (u<50) 

    // Get the distance D from the ray point to the scene geometry.
    // If it is less than a threshold, then we have intersected the scene,
    // otherwise increase the distance along the ray by most of l:
    if (fabs(l=D(o+d*(u+=l),n,m))<1e-4){

      // Ok, we have intersected! Now compute lighting, which consists
      // of three terms: a Lambertian term, a specular term, and ambient
      // occlusion.

      // s is specular lighting component
      f s=pow(U(n%!(L-d)),80),w;

      // update o to put it at the point of intersection
      o=o+d*u;

      // It's time to compute ambient occlusion now. At this point it
      // would be instructive to look at the linked PDF here:
      //
      //   http://iquilezles.org/www/material/nvscene2008/nvscene2008.htm
      //
      // Basically, we're going to use the distance function D to get an
      // idea of how much "clutter" there is as we march away from the 
      // intersecting point along the normal n.  
      //
      // Take 5 steps along the normal:
      //
      while (++c<6)  

        // OK, we are comparing two distances here:
        //
        //   .2*c is the distance along the normal from the point of
        //   intersection.
        //
        //   The D(...) subexpression is the distance to closest point
        //   in the scene. 
        //
        // If the point of intersection was the closest thing, then
        // the difference below should be zero and there is no
        // occlusion happening.
        //
        // However, if there's non-convex geometry in that region, the
        // D(...) thing will be less than the .2*c thing and we end up
        // contributing to a. Each time we take a step along the
        // normal, we reduce the "importance" of the AO computation by
        // a factor of 2 (occlusion at larger distances matter
        // exponentially less).
        //
        a -= (c/5-D(o+n*.2*c,d,w))/pow(2,c);

      // Do all the lighting now:
      //
      //   v(m,m,1) is the color (white or blue)
      //
      //   U(n%L)/3+.65 is a blend of 0.33 Lambertian and 0.65
      //   ambient, and modulates the color.
      //
      //   So then we take the modulated color, and use the specular
      //   term to linearly interpolate between that and white
      //   (totally fake specular lighting).
      //
      //   Finally we take that entire thing and darken it up with
      //   the ambient occlusion term that we computed above.
      //
      return (v(m,m,1)*(U(n%L)/3+.65)*(1-s) + v(1,1,1)*s)*a;

    }
  
  // Return values are overrated.

}

//////////////////////////////////////////////////////////////////////
// Main function just prints a header and iterates through the image 
// pixels.

int main() {


  // t holds the vertical pixel coordinate
  f t=-111;

  // Output the portable pixmap header (binary version).
  puts("P6 600 220 255");

  // For each row:
  while (++t<110) 

    // For each column:
    // s holds the horizontal pixel coordinate
    for (f s=-300;s<300;++s) {

      // Get the color of the pixel as a float with components in
      // [0,1] and scale it into [0,255].  View transformation? Never
      // heard of it.  Naw, let's just directly encode our camera
      // position and viewing direction as inline variables.
      // 
      v c = T(v(-2,4,25),
              !((!v(5,0,2)*s - 
                 !v(-2,73)*t)*.034 +
                v(10.25,-2,-25)))*255;

      // Output each channel as a character.
      putchar(c.x);
      putchar(c.y);
      putchar(c.z);

    } // for each column

}
