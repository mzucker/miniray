//////////////////////////////////////////////////////////////////////
// This is the "verbose" version of an obfuscated business card 
// sized raytracer. Adding comments and whitespace increases the
// size from 1294 bytes to 10127 bytes.
//
// Compile this with full optimization and pipe the output to a file
// named "mattz.pgm"

#include <stdio.h>
#include <math.h>

// Start by saving some space:
typedef float f;
typedef unsigned char u;
typedef int i;

//////////////////////////////////////////////////////////////////////
// A basic vector class. Note all vector operations are overloaded
// single character operators -- no "named" methods. Thanks, Bjarne!

struct v {

  // Just hold x,y,z.
  f x,y,z;

  // Construct with 1, 2, or 3 elements
  v (f a=0, f b=0, f c=0): x(a),y(b),z(c) {}

  // Multiplication by scalar.
  v operator*(f s) { return v(x*s,y*s,z*s); }

  // Dot product
  f operator%(v r) { return x*r.x+y*r.y+z*r.z; }

  // Vector addition
  v operator+(v r) { return v(x+r.x,y+r.y,z+r.z); }

  // Vector subtraction (defined in terms of multiplication by -1 and
  // addition to save space).
  v operator-(v r) { return *this+r*-1; }

  // Return a normalized version of this vector.
  v operator!() { return *this*(1/~*this); }

  // Return the length of this vector.
  f operator~() { return sqrt(*this%*this); }

} 

// The vector L holds the light direction in world space.  Yes, you
// can declare a variable in the same statement as a class
// declaration.
L(!v(-1,1,2));


//////////////////////////////////////////////////////////////////////
// A structure for distance queries that derives from the vector
// class. Given an arbitrary point in the world, store the closest
// point in the scene geometry, along with the distance from world
// point to scene point. This structure is updated several times
// as we test distances to various scene primitives.
//
// The underlying vector (base class) stores the arbitrary world
// point.

struct q: v {

  // Closest point in scene geometry
  v c;

  // Distance from world point to scene point
  f d;

  // Construct with a sufficiently large distance.
  q(v P): v(P), d(99) {}

  // Check to see if point n in the scene geometry is closer than
  // current closest point c. If so, set c and d.
  // 
  // It's fewer characters to return an int than a void, so we take 
  // the hit and don't return anything from this method (generates
  // a warning in g++, oh well).
  i operator()(v n) { 
    f e= ~(*this-n);
    if (e<d) { c=n; d=e; }
  }

};

//////////////////////////////////////////////////////////////////////
// Clamp any float to the [0,1] unit interval.

f U(f a) { return a<0?0:a>1?1:a; }

//////////////////////////////////////////////////////////////////////
// Test the distance from the point p to the scene, and store the
// scene normal (normalized difference between p and closest point in
// the scene) in n.

f D(v p, v& n) {

  // Initialize our distance query h.
  q h(p);

  // Encode the letters "mattz", see below for explanation:
  u d[] = "`@@B@D$\x82$\x86\xe3\x8d`H\x81Jc)B\x96\x81Nc-B\x9e`1c1`q";

  // Interpret bytecodes by iterating over d in 2-byte chunks.
  for (u* c=d;*c;c+=2) {

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
    i o=c[1]>>5, a=*c>>5; 
    f x=c[1]&31, y=*c&31; 

    // Check if the high bit of o is set.

    if (o<4) { // High bit not set.

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
      h(k+d*U((h-k)%d/(d%d)));

    } else { // High bit is set.

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
      f P=M_PI, l=-P*(a&2)/2, u=P*(a&1), 
        r=1+.5*(a/4), t=atan2(h.y-y/2,h.x-x/2);

      // Clamp t to the range given.
      t = t<l?l:t>u?u:t;

      // Compute the point along the arc closest to p (just given by
      // the angle t and radius r), and feed it to the distance query.
      h(v(x/2+r*cos(t),y/2+r*sin(t)));

    } // Done with current primitive

  } // Looping through primitives

  // Find the projections of point p onto the wall and floor planes,
  // and feed them to the distance query.
  n=v(0,0,1); h(h-n*(h%n+.9));
  n=v(0,1); h(h-n*(h%n+.9));

  // Compute the normal n.
  n=!(p-h.c);

  // Return the distance -- we subtract off a radius of .45 to give
  // everything some dimension (otherwise all primitives would be
  // infinitely thin and we would just see the planes).
  return h.d-.45;

}

//////////////////////////////////////////////////////////////////////
// Trace a ray from origin o along direction d, and return the scene
// brightness at the intersection. Instead of analytic ray tracing,
// this is actually "sphere tracing" by using the distance to closest
// point in scene to lower-bound how far we can step along the ray.

f T(v o, v d) {

  // u holds the distance along the ray, l holds the distance to
  // closest object.
  f u=0, l=0;

  // n holds the normal (normalized difference of ray point minus
  // closest scene point).
  v n;

  // March along the ray:
  while (u<50) 

    // Get the distance D from the ray point to the scene geometry.
    // If it is less than a threshold, then we have intersected the scene,
    // otherwise increase the distance along the ray by most of l:
    if (fabs(l=D(o+d*(u+=.9*l),n))<1e-4){

      // Ok, we have intersected. It's time to compute lighting and
      // ambient occlusion now. At this point it would be instructive
      // to look at the linked PDF here:
      //
      //   http://iquilezles.org/www/material/nvscene2008/nvscene2008.htm
      //
      // Basically, we're going to use the distance function D to get an
      // idea of how much "clutter" there is as we march away from the 
      // intersecting point along the normal n.  
      //
      // Our variables:
      //
      //   c is just a counter that counts steps along the normal.
      //
      //   a is going to track our ambient occlusion (lower value =
      //   more occluded)
      //
      //   s is the specular contribution (Phong shading model).
      //
      f c=0, a=1, s=pow(U(n%!(L-d)),80);

      // We reset o to be the point of intersection instead of the ray
      // origin from here on out.  Also, we're clobbering d in the call
      // to D below.
      o=o+d*u;

      // Take 5 steps along the normal:
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
        a -= .8*(.2*c-D(o+n*.2*c,d))/pow(2,c);
      
      // Do the lighting and modulate the entire lighting computation by a:
      //
      // Inside the parentheses:
      //
      //   Basically if s is zero, we are totally lambertian with a minimum
      //   brightness of .65 and a maximum brightness of 0.95
      //
      //   If s is one, then we are totally white.
      //
      //   We use the specular contribution to modulate between
      //   lambertian and total whiteness.
      //
      return ((.3*U(n%L)+.65)*(1-s)+s)*a;

    } // End marching along the ray
  
  // The ray missed the scene entirely (shouldn't happen), so return
  // zero brightness. Probably could just remove this statement
  // and pick up another warning.
  return 0;

}

//////////////////////////////////////////////////////////////////////
// Main function just prints a header and iterates through the image 
// pixels.

i main() {

  // Output the portable graymap header (binary version).
  puts("P5 600 220 255"); // * size encoded into the string

  i t=-111; // * t holds the vertical pixel coord.

  while (++t<110) // * 

    for (i s=-300;s<300;++s) { // * s holds the horiz. coord.

      // Get the brightness of the pixel as a float in 0,1 and output
      // the character.  View transformation? Never heard of it.  Naw,
      // let's just directly encode our camera position and viewing
      // direction as inline variables. 
      //
      // If you wanted to resize the output image, you'd have to change
      // all the marked lines (*) to get the output to scale uniformly.
      putchar(255*T(v(-2,4,25),
                    !((!v(5,0,2)*s - 
                       !v(-2,73)*t)*.034 + // * .034 is a pixel scaling factor
                      v(10.25,-2,-25))));
    }

  // If we were good citizens, we'd return something here. Oh well.

}
