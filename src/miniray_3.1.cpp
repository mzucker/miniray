//////////////////////////////////////////////////////////////////////
// This is the "verbose" version of an obfuscated business card 
// sized raytracer. 
//
// Compile this with full optimization and pipe the output to a file
// named "mattz.ppm"

#include <stdio.h>
#include <math.h>

// Start by saving some space:
#define O operator
#define E return

typedef float f;

f H=.5, Z=.33, Y=Z+Z, S=-1, I, y=-111;

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
  v operator!() { v&t=*this; return t*pow(t%t,-H); }

  // Vector addition
  v operator+(v r) { return v(x+r.x, y+r.y, z+r.z); }

}  

// The vector L holds the light direction in world space. Defining it
// right after the class declaration saves us a character or two later
// on.
  L=!v(S,1,2),
  W(1,1,1),
  P, C, M, N, A;
  

//////////////////////////////////////////////////////////////////////
// Clamp any float to the [0,1] unit interval.

f U(f a) { return a<0?0:a>1?1:a; }

//////////////////////////////////////////////////////////////////////
// A helper function for distance queries. The scene is broken up into
// a number of primitives. For some point P, we cycle through all the
// primitives to see which one is closest to P, and this function is
// called once per primitive. Variables in this function:
//
//  c: the closest point on the current primitive to P
// 
//  C: the closest point to P on any primitive found so far
//  I: the squared distance between P and c
// 
// The implementation is dead simple: if the distance between p and c
// is less than I, we simply update C and I

f Q(v c) {

  M = P+c*S;
  f d = M%M;

  return d < I ? C=c, I=d : 0;

}

//////////////////////////////////////////////////////////////////////
// Test the distance from the point p to the scene, and store the
// scene normal (normalized difference between p and closest point in
// the scene) in n, and the "material" of the closest primitive in m.
//
f D(v p) {

  // P, I are the variables associated with the distance query
  // function above.
  //
  f x=0;

  I=99;
  P=p;

  // Encode the letters "mattz", see below for explanation:
  // Interpret bytecodes by iterating over string in 2-byte chunks.
  for (const char* b="BCJB@bJBHbJCE[FLL_A[FLMCA[CCTT`T"; *b; ++b) {

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
    x+=*b/4&15;
    int o=*b&3, a=*++b&7, y=*b/8&7;

    v k(x,y), d(o%2*a,o/2*a);

    if (o) 

      // The current primitive is a line segment extending
      // from (x,y) to (x+dx, y+dy), where:
      //
      //   dx = a if the low bit of o is set, otherwise zero.
      //
      //   dy = a if the middle bit of o is set, otherwise zero.
      //
      // Here, k stores the point (x,y), d stores the vector (dx,dy)
      //

      // Compute the closest point along the line segment and
      // feed it to the distance query. 
      Q(k+d*U((p+k*S)%d/(d%d)));

    else {

      // The current primitive is an arc centered around
      // (x,y) with starting angle l, ending angle u,
      // and radius r, where
      //
      //   l = -M_PI if the high bit of a is set, otherwise 0.
      //   
      //   u = M_PI if the middle bit of a is set, otherwise 0.
      //
      //   r = 1.5 if the low bit of a is set, otherwise 1.
      //
      // Also compute t, the angle of the point p with respect to the
      // arc's center.
      //
      f t=atan2(p.y-y*H,p.x-x*H),
        l=a/4%2*-M_PI, u=a/2%2*M_PI;
      
      // Clamp t to the range given.
      t = t<l?l:t>u?u:t;

      // Compute the point along the arc closest to p (just given by
      // the angle t and radius r), and feed it to the distance query.
      Q(k*H+v(cos(t),sin(t))*(a%2*H+1));

    } 


  } // Looping though primitives

  // Find the projections of point p onto the floor plane and feed it
  // to the distance query. If it is the closest, evaluate the floor
  // texture and assign the material accordingly; otherwise assign the
  // letter material.
  M = Q(v(p.x,-.9,p.z)) ?
    (int((p.x+64)/8)^int((p.z+64)/8))&1 ? Y : W : v(Y,Y,1);

  // Compute the normal n.
  N=P+C*S;

  // Return the distance -- we subtract off a radius of .45 to give
  // everything some dimension (otherwise all primitives would be
  // infinitely thin and we would only see the planes).
  return sqrt(I)-.45;

}

v R(v o, v d, f z) {

  // Scalar variables:
  //
  //   u: distance along the ray at current point
  //   l: distance to closest object at current point
  //   m: primitive "material"
  //   a: ambient occlusion parameter
  //
  // March along the ray: Get the distance D from the current ray
  // point to the scene geometry.  If it is less than a threshold,
  // then we have intersected the scene, otherwise increase the
  // distance along the ray by l:

  for (f u=0, l=1, i=0, a=1; u<97; u+=l=D(o+d*u))
        
    if (l<.01) {

      v p=M, n=!N;
          
      // Ok, we have intersected! Now compute lighting, which consists
      // of three terms: a Lambertian term, a specular term, and ambient
      // occlusion.
          
          
      // It's time to compute ambient occlusion now. At this point it
      // would be instructive to look at the linked PDF here:
      //
      //   http://iquilezles.org/www/material/nvscene2008/nvscene2008.htm
      //
      // Basically, we're going to use the distance function D to get an
      // idea of how much "clutter" there is as we march away from the 
      // intersecting point along the normal n.  
      //
      // First update o to put it at the point of intersection, then 
      // Take 5 steps along the normal:
      for (o=o+d*u; ++i<6;)  

        // OK, we are comparing two distances here:
        //
        //   i*.3 is the distance along the normal from the point of
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
        // D(...) thing will be less than the i*.3 thing and we end up
        // contributing to a. Each time we take a step along the
        // normal, we reduce the "importance" of the AO computation by
        // a factor of 2 (occlusion at larger distances matter
        // exponentially less).
        //
        a -= U(i/3-D(o+n*i*.3))/pow(2,i);

      // Do all the lighting now:
      //
      //   v(m,m,1) is the color (white or blue)
      //
      //   U(n%L)/3+.65 is a blend of 0.33 Lambertian and 0.65
      //   ambient, and modulates the color.
      //
      //   Finally we take that entire thing and darken it up with
      //   the ambient occlusion term that we computed above.
      p = p*(U(n%L)*Z+Y)*a;

      if (z) 
        p = p*Y + R(o+n*.1,d+n*-2*(d%n),z-1)*Z;

      // Compute a specular term and use it to linearly interpolate
      // between the surface color and white; this is physically
      // implausible, but doesn't require clamping (as does, for
      // instance, the Phong shading model).
      u=pow(U(n%!(L+d*S)),40);

      return p + p*-u + W*u;
      
    } // hit

  // Didn't hit anything, return the sky color.
  z=d.z*d.z;
  return v(z, z, 1);

}
      


//////////////////////////////////////////////////////////////////////
// Main function just prints a header and iterates through the image 
// pixels.

int main() {

  // Output the portable pixmap header (binary version).
  // Then, for each row:
  for (puts("P6 600 220 255"); ++y<110;) 

    // For each column:
    // x holds the horizontal pixel coordinate
    for (f x=-301;++x<300; putchar(A.z))

      // Trace a ray from origin o along direction d, and compute the
      // scene color at the intersection. Instead of analytic ray
      // tracing, this is actually "sphere tracing" by using the
      // distance to closest point in scene as a lower-bound of how
      // far we can step along the ray.

      putchar((A = R(v(-2,4,25),
            !((!v(5,0,2)*x +
               !v(-2,73)*-y) +
              v(301,-59,-735)),
                     2)*255).x),

        putchar(A.y);
      
      // Then output each channel of the pixel as a character. (see
      // for loop increment above)

  
}

