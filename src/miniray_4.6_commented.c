// This file is identical to the final zucker.c entry, except for
// variable and member identifers and formatting.

#include <stdio.h>
#include <math.h>

// We start with this big ugly program data string. Its formatting is
// as follows:
//
//   Bytes 0-9 are "IOCCC 2011" xor-ed with 5 -> "LJFFF%7544"
//   Bytes 10-24 are "P6 600 220 255\n" plus 40 -> "x^H^XXHZZXHZ]]2"
//   Bytes 25 on are the font encoding table.
//
// The font encoding table may be decoded a byte at a time:
//
//   - A byte less than 42 (ASCII *) is a START token that indicates
//     the start of a new glyph, and which also embeds the width of
//     the glyph by its difference from 34 (ASCII "); hence byte 35
//     (ASCII #) begins a character of width 1 and byte 39 (ASCII ')
//     begins a character of width 5.
//
//   - If the byte immediately following a START token is equal to 40
//     (ASCII open paranethesis), it is a SETCUR token and the byte
//     following the SETCUR token indicates the ASCII value of the
//     glyph being emitted.
//
//   - If there is no SETCUR token immediately after a START token,
//     the ASCII value of the glyph being emitted is one greater than
//     the previous glyph that was emitted.
//
//   - Beyond an optional SETCUR / ASCII value pair, a glyph may
//     contain one or more strokes defined by two-byte opcodes, first
//     byte b0 and then byte b1, where:
//
//       * the ARGUMENT is determined by the lowest 5 bits of b0
//       * the OPCODE is determined by the next highest 2 bits of b0
//       * the YCOORD is determined by the lowest 3 bits of b1
//       * the XCOORD is determined by the next highest 2 bits of b1
//
//     There are 3 permissible OPCODE values. Each one interprets its
//     ARGUMENT, XCOORD, and YCOORD differently:
//
//       * OPCODE 3 denotes an ARC. The ARGUMENT of the ARC is split
//         up into A0 (upper 3 bits) and A1 (lower 2 bits). A0 denotes
//         the starting angle in multiples of 90 degrees, and A1
//         denotes the angular range in multiples of 90
//         degrees. XCOORD and YCOORD specify the center of the ARC.
//
//       * OPCODE 2 denotes a LINE. The ARGUMENT of the line is split
//         into a DELTAX (upper 2 bits) and DELTAY (lower 3 bits).
//         XCOORD and YCOORD specify the lower-left endpoint of the
//         LINE and the upper-right is reached by adding DELTAX and
//         DELTAY.
//
//       * OPCODE 1 denotes an NLINE, which is identical to a LINE,
//         except that the DELTAX is interpreted to have negative
//         value. This is necessary for diagonal lines such as the
//         bottom leg of a 'k' or the center of an 's' which point
//         down and to the right.
//
//   - Instructions for a glyph are terminated by a new START token or
//     by a terminating zero.
//
// Hence, the start of the font encoding table "#( #@@DA" encodes two
// glyphs; the first glyph is width 1 (because of the #) and it is the
// space character (because of the "( " immediately following). the
// second character is also width 1 (because of the #) and it contains
// the two strokes specified by "@@" and "DA".

char* progdata = "LJFFF%7544x^H^XXHZZXHZ]]2#( #@@DA#(.@@%(0CAaIqDCI$IDEH%P@T@qL%PEaIpBJCAI%KBPBEP%CBPEaIqBAI%CAaIqBqDAI%U@PE%AAaIqBcDAI%ACaIaCqDCI%(aHCcIpBBH%E@aIqBAI%AAaIqB%AAaIqBEH%AAPBaIqB%PCDHxL%H@hIcBBI%E@qJBH#C@@D%aIBI@D%E@QB2P#E@'C@qJBHqJBH%C@qJBH%AAaIqBAI%C@cJ%cJCH%C@qJ%aIqB1I%PCDI`I%BAaICH%KH+@'JH+@KP*@%S@3P%H@ABhIaBBI%P@S@PC#", *fontptr, *textptr;

// This was a one-letter typedef in the actual program which obviously
// saved a bit of space.
typedef float real;

// Clamp a real number to [0,1] range
real clamp(real a) {
  return a < 0 ? 0 : a > 1 ? 1 : a;
}

// Nice vec3 structure
typedef struct {
  real c,a,t; // I like cats.
} vec3;

// All of these are pretty much straightforward except for
// vecop_buffer, which holds the font rendering instructions decoded
// from the font table (see below).
vec3 ones={1,1,1}, zeros={0,0,0}, vecop_buffer[99], point,
  closest_point, objpoint_or_material, normal, vec3_out, pix_color,
  sample_color, ray_dir, march_point;

// Note that vec3_out always holds the last constructed vec3, which is
// duplicated by the return-by-value.
vec3 make_vec3(real t, real a, real c) {
  // Order scrambled for obfuscation's sake
  vec3_out.c=t;
  vec3_out.t=c;
  vec3_out.a=a;
  return vec3_out;
}

// A bunch of ints, along with an int function (putchar)
int minus_one=-1, buffer_offset=0, buffer_length, image_row=-111,
  (*output)(int) = putchar, forty=40, xormask_or_quality=5,
  lo_or_found, range_or_curglyph, fbyte_or_aacnt, hi_or_started=0,
  bounces, hit;

// Why have separate functions for add, subtract, or multiply when
// this one does all 3?
//
//  a + b = add_scaled(a, b, 1)
//  a - b = add_scaled(a, b, -1)
//  0.5 * a = add_scaled(zero, a, 0.5)
//
vec3 add_scaled(vec3 a, vec3 b, real c) {
  // Order scrambled for obfuscation's sake
  return make_vec3(a.c+b.c*c, a.a+c*b.a, b.t*c+a.t);
}

// A bunch of real variables and constants
real half=.5, twothirds=.66, closest_dist_squared, text_width=0,
  dot_out_or_tmp, angle_or_width, raydist_or_brightness,
  curdist_or_specular, ray_contribution;

// Note dot_out_or_tmp always gets set to most recent dot product.
real dot(vec3 a, vec3 b) {
  // Order scrambled for obfuscation's sake
  return dot_out_or_tmp = a.t*b.t + b.c*a.c + a.a*b.a;
}

// This is called for every vecop pair in the vecop_buffer. The point
// global has been set to the query point, and objpoint_or_material
// has been set to the closest point in the current primitive.
real update_distance_query() {

  // compute (point - objpoint_or_material)
  return add_scaled(point, objpoint_or_material, minus_one),

    // compare squared length of result to previous closest dist
    dot(vec3_out,vec3_out) < closest_dist_squared ?

    // if smaller, update closest_point and closest_dist_squared
    closest_point=objpoint_or_material,
    closest_dist_squared=dot_out_or_tmp :

    // otherwise do nothing
    0;
}

// Normalize by abusing scaled add function:
//
//   n / ||n|| = zero + n * pow(||n||, -0.5)
//
vec3 normalize(vec3 a) {
  return add_scaled(zeros, a, pow(dot(a,a),-half));
}

// For the point input, this will compute and return the distance to
// the closest scene primative. It will also update the
// objpoint_or_material to reflect the material of the closest object
// (blue for text, red or white for floor).
real dist_to_scene(vec3 point_or_vecop) {

  // This loop initializes global closest_dist_squared to a big number
  // and point to the function's argument.  We will iterate over pairs
  // of vectors in the vecop_buffer using buffer_offset up to the
  // given buffer_length.
  for (closest_dist_squared=forty,
         point=point_or_vecop, buffer_offset=minus_one; 

       // The first item in the vecop_buffer pair becomes objpoint
       // (arc center or line endpoint). The second item denotes line
       // displacement or arc angle. Note we also offset the
       // x-coordinate of the objpoint to approximately center the
       // text. The last thing we do here is bail out of the loop if
       // we are past end-of-buffer.
       objpoint_or_material = vecop_buffer[++buffer_offset],
         point_or_vecop =
           vecop_buffer[objpoint_or_material.c+=8.8-text_width*.45,
                        ++buffer_offset],
         buffer_offset<=buffer_length;

       // After each loop iteration, we should update the distance
       // query but of course this doesn't get evaluated until after
       // the code below.
       update_distance_query()) 

    // Currently objpoint is arc center or line endpoint.  We must
    // update it to the closest point on the primitive (arc or line)
    // to the current point. First we check if we are arc (op.t!=0) or
    // line (op.t=1):
    objpoint_or_material = point_or_vecop.t ?

      //////////////////////////////////////////////////
      // We are an arc

      // Quantize angles to 90 degree increments
      dot_out_or_tmp = M_PI*half,

      // Get the actual angle and divide by 90 degrees.  We will now
      // need to clamp the angle to deal with the arc endpoints
      // nicely.
      angle_or_width=atan2(point.a-objpoint_or_material.a,
                           point.c-objpoint_or_material.c)/dot_out_or_tmp,

      // op.c encodes lower bound of angle
      lo_or_found=point_or_vecop.c-2,

      // op.a encodes upper bound of angle
      range_or_curglyph=point_or_vecop.a+1,

      // Get the upper end of the range
      hi_or_started=lo_or_found+range_or_curglyph,

      // Now clamp! Note we scale overall result by 90 degrees
      angle_or_width = dot_out_or_tmp*(
         // did we wrap past the upper end of the range?
         angle_or_width>hi_or_started+half*range_or_curglyph ?
         // if so go low
         lo_or_found :
         // otherwise greater than upper end?
         angle_or_width > hi_or_started ?
         // then go hi
         hi_or_started :
         // did we wrap past the lower end of the range?
         angle_or_width<lo_or_found-half*range_or_curglyph ?
         // go hi
         hi_or_started :
         // otherwise less than lower end?
         angle_or_width<lo_or_found ?
         // go lo
         lo_or_found :
         // no clamp needed
         angle_or_width),

      // Now we can finally offset the objpoint (arc center) by angle
      // (note radius = 1), which is convenient.
      add_scaled(objpoint_or_material,
                 make_vec3(cos(angle_or_width), sin(angle_or_width), 0), 1)
      
      :

      //////////////////////////////////////////////////
      // Nope, not arc, a line segment.

      // In this case op is just the displacement of endpoint from
      // startpoint so we can compute the nearest point along the line
      // using the standard formula.
      add_scaled(objpoint_or_material, point_or_vecop,
                 clamp(dot( add_scaled(point, objpoint_or_material, minus_one),
                            point_or_vecop) /
                       dot(point_or_vecop, point_or_vecop) ) );

  //////////////////////////////////////////////////
  // Done with for loop.

  // Now we need to check distance to the floor, which exists
  // everywhere at a coordinate of y = -0.9. here we update the
  // objpoint by copying the point and setting y.
  objpoint_or_material=point;
  objpoint_or_material.a = -.9;

  // Here we are creating a nice checkerboard texture by XOR'ing the x
  // and z coordinates mod 8.
  lo_or_found = point.c/8+8;
  lo_or_found ^= range_or_curglyph=point.t/8+8;

  // Finally, we are going to save the material. First we update the
  // distance query based upon the floor objpoint:
  objpoint_or_material = update_distance_query() ?

    // if the query updated, we are the floor, so red or white
    lo_or_found&1 ? make_vec3(twothirds,0,0) : ones :

    // otherwise the query didn't update so we are blue.
    make_vec3(twothirds,twothirds,1);

  // Finally we return the distance to closest point, offset by 0.45
  // (to give the primitives some thickness)
  return sqrt(closest_dist_squared)-.45;

}

// Finally our main function
int main(int argc_or_image_col, char** argv) {

  //////////////////////////////////////////////////
  // Step 1: fill up the vecop_buffer with the correct primitives for
  // the text to display by parsing the font table.
  //
  // The outer loop here is over characters to print, which come
  // either from argv[1] (if argc > 1), or from progdata table above,
  // in which case they have to be XOR-ed with 5 to get the actual
  // text.
  
  for (// Was a string provided on the command line?
       textptr = argc_or_image_col>1 ?
         // If so, initialize textptr from argv[1] and clear XOR mask
         1[xormask_or_quality=0, argv] :
         // else, initialize textptr from progdata.
         progdata;

       // Go until terminating 0 or text too wide
       *textptr && text_width<24;

       // Increment textptr each iteration.
       ++textptr)

    // Inner loop is over the font table encoded in progdata (see
    // explanation at top). For each text character, we need to try to
    // find the corresponding glyph in the font and push all of its
    // strokes into the vecop_buffer, two vectors at a time.
    
    for (// Initialize found to false
         // Initialize range_or_curglyph to zero ???
         lo_or_found=range_or_curglyph=0,
           
           // Start 10 characters into the text (after "ioccc 2011")
           // but we increment fontptr before dereferencing it because
           // gross, so just offset by 9 here.
           fontptr=progdata+9;

         // Read the next byte from the font table (stop if we
         // hit a terminating 0). 
         (fbyte_or_aacnt=*++fontptr) &&

           // Keep going as long as one of these holds:
           //
           //   - lo_or_found is zero (haven't found char)
           //   - fbyte_or_aacnt >= 40 (lower ones are space !"#$%&')
           //
           // This means that once lo_or_found is nonzero and we hit a
           // START token or a null zero, we are done.
           //
           // Since logical AND is short-circuiting, the update to
           // dot_out_or_tmp only happens if found is true and the
           // current character is greater than or equal to 40.
           //
           // Once that happens, we exit the loop because all three
           // conditions inside these parens are true, making the
           // entire thing false when NOT-ed.
           !(lo_or_found && fbyte_or_aacnt<forty &&
             (dot_out_or_tmp=text_width+=angle_or_width));

         // See if we have found our glyph yet
         lo_or_found ?

           //////////////////////////////////////////////////
           // We have found it -- we are in the current glyph.

           // The current byte should hold an OPCODE and and ARGUMENT.
           // Stash the OPCODE into lo_or_found and increment
           // fontptr. The ARGUMENT should be available by inspecting
           // fbyte_or_aacnt which still holds the byte that fontptr
           // was pointing to.
           lo_or_found=*fontptr++/32,

           // Now get the XCOORD (bits 34) and YCOORD (bits 210) from
           // second byte of stroke instructions. The XCOORD gets
           // added to the x-accumulator (dot_out_or_tmp) and then the
           // vector (XCOORD, YCOORD, 0) is pushed into the
           // vecop_buffer.  This is either the start of a line
           // segment or the center of an arc.
           buffer_offset++[vecop_buffer] =
              make_vec3(dot_out_or_tmp+=*fontptr/8&3,*fontptr&7,0),

           // Time to push the second vector into the vecop_buffer.
           // In the case of an arc (OPCODE == 3), this is starting
           // angle and range, or in the case of a line segment
           // (OPCODE == 2 or OPCODE == 1) this is dx, dy. We need to mirror
           // the x-coordinate if OPCODE == 1.
           //
           // In any event, all of this information is hanging out in
           // the ARGUMENT, which is the lower 5 bits of
           // fbyte_or_aacnt (first byte of stroke instruction pair).
           //
           // Here we also update buffer_length here to be used later
           // in dist_to_scene.
           vecop_buffer[buffer_length=buffer_offset++] =
             make_vec3((fbyte_or_aacnt/8&3)*(lo_or_found<2?minus_one:1),
                       (fbyte_or_aacnt&7)+1e-4,
                       lo_or_found>2),

           // Just a NOP because the ternary operator we're inside of
           // here is of type int.
           1

           :

           //////////////////////////////////////////////////
           // Glyph not found yet.

           // Try to update our found variable...
           (lo_or_found =

            //////////////////////////////////////////////////////////////////////
            // Update cur glyph according to current font table byte.
            
            (range_or_curglyph =

             // Subtract 40 from current font table byte. Less than zero?
             (fbyte_or_aacnt-=forty) < 0 ?

             //////////////////////////////////////////////////
             // Yes, less than zero, so this is a START token.
             
             // Glyph width given by byte - 34 = byte - 40 + 6
             angle_or_width=fbyte_or_aacnt+6,
             
             // Increment cur glyph and mark state as started.
             hi_or_started=range_or_curglyph+1

             :

             //////////////////////////////////////////////////
             // Was font table byte nonzero after subtracting 40?
             fbyte_or_aacnt ?

             // Yes, nonzero. 
             ( // Now see if we have started seeing font table
               // bytes yet, or if we are still going thru the
               // PPM header.
               hi_or_started?
               // Yes, we have started, so NOP.
               0
               :
               // Not started, so emit the current fbyte_or_aacnt (to
               // generate PPM header)
               output(fbyte_or_aacnt),
               // Comma says ignore results of previous ternary
               // operator and leave range_or_curglyph unchanged
               range_or_curglyph
               )

             :

             //////////////////////////////////////////////////
             // Font table byte was 40 (SETCUR token), so set
             // current glyph to next byte in font table.
             *++fontptr)

            //////////////////////////////////////////////////////////////////////
            // Compare the newly-updated cur glyph to...

            ==

            // The current text character, OR'ed with 32 to put in
            // range of 32-63 or 96-127 (forces lowercase), and XOR'ed
            // with mask to deobfuscate "ioccc 2011" from progdata.
            ((*textptr|32)^xormask_or_quality)

            &&

            ////////////////////////////////////////
            // Need to clear found bit whenever we hit a SETCUR token.
            1[fontptr]-forty)


         ); // Empty for loop

  //////////////////////////////////////////////////
  // Step 2: Generate the dang image.
  //
  // All of the techniques here are based upon the PDF presentation at
  // http://iquilezles.org/www/material/nvscene2008/nvscene2008.htm
  //
  // Iterate over image rows. Note xormask_or_quality gets value 0 in
  // preview mode, and 3 in high-quality mode.
  for (xormask_or_quality=3*(argc_or_image_col<3); ++image_row<110; )

    // Iterate over image columns. 
    for (argc_or_image_col=-301;

         // Initialize the pixel color to zero, 600 cols total
         pix_color=zeros, ++argc_or_image_col<300;

         // Output pixel after each iteration.
         output(pix_color.c),output(pix_color.a),output(pix_color.t))

      // Iterate over AA samples: either 1 (preview) or 4 (high-quality).
      for (fbyte_or_aacnt=minus_one; ++fbyte_or_aacnt<=xormask_or_quality;)

        // Shade this sample. This for loop iterates over the initial
        // ray as well as reflection rays (in high quality mode).
        
        for (// Start marching at the shared ray origin 
             march_point=make_vec3(-4,4.6,29),

               // Starting direction is a function of image row/column
               // and AA sample number.
               ray_dir=normalize(
                 add_scaled(
                   add_scaled(
                     add_scaled(zeros, normalize(make_vec3(5,0,2)),
                                argc_or_image_col + argc_or_image_col +
                                fbyte_or_aacnt/2 ), 
                     normalize(make_vec3(2,-73,0)),
                     image_row+image_row+fbyte_or_aacnt%2),
                   make_vec3(30.75,-6,-75),
                   20) ),

               // The initial ray contribution is 255 for preview mode
               // or 63 for each AA sample in high-quality mode
               // (adding 4 of them gets you 252 which is close
               // enough).
               //
               // Also, here bounces is initialized to 3 in
               // high-quality mode or 0 in preview mode.
               ray_contribution=hit=
                 255-(bounces=xormask_or_quality)*64;

             // The bounces variable acts as a counter for remaining
             // bounces; at the start, bounces is non-negative and hit
             // is non-zero so the loop always runs at least once. It
             // will stop when hit is 0 or hit is 1 and bounces is -1.
             hit*bounces+hit;

             // After each iteration (reflection), ray contribution
             // scales by 0.5.
             ray_contribution*=half)

          {
            
            // Perform the actual ray march using sphere tracing:
            for (// Initialize ray distance, current distance, and hit to 0
                 raydist_or_brightness=curdist_or_specular=hit=0;

                 // Keep going until hit or ray distance exceeds 94 units.
                 // Note ray distance always incremented by current.
                 !hit && 94 > (raydist_or_brightness+=
                               
                   // Obtain distance to scene at current point
                   curdist_or_specular=dist_to_scene(
                                                      
                     // Update current point by moving by current
                     // distance along ray direction
                     march_point = add_scaled(
                       march_point,
                       ray_dir,
                       curdist_or_specular)));

                 // After each ray update, set hit=1 if current
                 // distance to scene primitive is less than 0.1
                 hit=curdist_or_specular<.01);

            // Done with ray marching!
            //
            // Now point is equal to march_point, closest_point holds
            // the closest point in the scene to the current ray
            // point, and objpoint_or_material holds the material
            // color of the closest object.

            // Now fake ambient occlusion loop (see iq's PDF for explanation):
            for (// Compute scene normal at intersection
                 normal = normalize(add_scaled(point,closest_point,minus_one)),
                   // This is actually included here to initialize the
                   // sky color below (gross).
                   dot_out_or_tmp = ray_dir.t*ray_dir.t,
                   // Also used below but initialized here.
                   sample_color = objpoint_or_material,
                   // Start at full brightness
                   raydist_or_brightness=1;

                 // 5 iterations if we hit something, 0 if not (saves
                 // wrapping for loop in if statement).
                 ++curdist_or_specular<6*hit;

                 // AO with exponential decay 
                 raydist_or_brightness -=
                   clamp(curdist_or_specular / 3 -
                     dist_to_scene(
                      add_scaled(march_point, normal, curdist_or_specular/3)))
                   / pow(2,curdist_or_specular));

            // AO has been computed, time to get the final color of
            // this ray sample. Note sample_color has been initialized
            // to material of closest primative above.

            sample_color = hit ? // Did this ray hit?

              //////////////////////////////////////////////////
              // Yes, the ray hit.

              // Get the Blinn-Phong specular coefficient as dot
              // product between normal and halfway vector, raised to
              // high power.
              curdist_or_specular =
                pow(clamp(dot(normal,
                  // normalize halfway vector
                  normalize(
                    // create halfway vector
                    add_scaled(
                      // objpoint_or_material is now light direcection
                      objpoint_or_material=normalize(make_vec3(minus_one,1,2)),
                      ray_dir,
                      minus_one)))),
                  // raised to the 40th power
                  forty),

              // Mix in white color for specular
              pix_color = add_scaled(pix_color, ones,
                                     ray_contribution*curdist_or_specular),

              // Take the brightness computed during AO and modulate
              // it with diffuse and ambient light.
              raydist_or_brightness *=
                // Diffuse - objpoint_or_material is light direction
                clamp(dot(normal, objpoint_or_material))*half*twothirds +
                // Ambient
                twothirds,

              // Modulate ray_contribution after hit 
              ray_contribution *= bounces-- ? // Are there any bounces left?
              
                // Yes, there are bounces left, so this hit should
                // account for 2/3 of the remaining energy (the next
                // will account for the final 1/3). We need to remove
                // the additive component already taken by specular,
                // however.
                twothirds - twothirds * curdist_or_specular :

                // No, there are no bounces left, so just use up
                // all the energy not taken by specular.
                1-curdist_or_specular,

              // Now after all of that, we're actually going to leave
              // sample_color unchanged (i.e. whatever closest
              // primitive material color was).
              sample_color
              :

              //////////////////////////////////////////////////
              // Nope, ray missed. Remember when we initialized
              // dot_out_or_tmp to contain z^2 above? We now use that
              // to shade the sky, which gets white along the +/- z
              // axis, and blue elsewhere.
              make_vec3(dot_out_or_tmp, dot_out_or_tmp, 1);

            // Add the weighted sample_color into the pixel color. 
            pix_color = add_scaled(pix_color,
                                   sample_color,
                                   ray_contribution*raydist_or_brightness);

            // Pop out from the object a bit before starting to march
            // the reflection ray so we don't immediately detect the
            // same intersection that we're on.
            march_point = add_scaled(march_point,normal,.1);

            // Update the ray direction to be the reflection direction
            // using the usual calculation.
            ray_dir = add_scaled(ray_dir,normal,-2*dot(ray_dir,normal));

          }


  return 0;

}
