#include <stdio.h>
#include <math.h>
#define _ return

typedef float real;
real half=.5, third=.33, twothird=.66, closestdist, ycoord=-111, xoffset=0, tempreal, width, objx, objy, cursorx;

real clamp(real a) { return a<0?0:a>1?1:a; }

typedef struct { real xx,yy,zz; } vec;
real dot(vec a, vec b) { return a.zz*b.zz+a.xx*b.xx+a.yy*b.yy; }

vec lightpos, white={1,1,1}, blue={.7,.7,1}, zerovec={0,0,0}, querypos, closest, closestmat, normal, objpos, objdir, tempvec;

vec makevec(real t, real a, real c) { tempvec.xx=t; tempvec.yy=a; tempvec.zz=c; return tempvec; }

vec addscale(vec a, vec b, real c) { return makevec(a.xx+b.xx*c, a.yy+b.yy*c, a.zz+b.zz*c); }

real updatequery(vec c, vec m) {
  tempvec = addscale(querypos,c,-1);
  tempreal = dot(tempvec,tempvec);
  return tempreal<closestdist ? closest=c,closestmat=m,closestdist=tempreal:closestdist;
}

char* data = "%(aHCcIhBBH%E@aIiBAI%AAaIiB%AAaIiBEH%AAPBaIiB%PCDHlL%H@dIcBBI%E@iJBH#C@@D%aIBI@D%E@QB2B#E@'C@iJBHiJBH%C@iJBH%AAaIiBAI%C@cJ%cJCH%C@iJ%1BaIiB%PCDI`I%BAaICH%+CKH'+CJH*JKH%S@3C%H@ABdIaBBI%P@S@PC#( #(.@@#(!@@DA%(0CAaIiDCI$IDEH%P@T@iL%PEaIhBJCAI%KBPBEP%CBPEaIiBAI%CAaIiBiDAI%U@PE%AAaIiBcDAI%ACaIaCiDCI\0ljfff%7544",

  buf[999], *bufcur=buf, *datacur, *strcur, forty=40, op, arg, curchar, aamask=5;

real dquery(vec point) {

  cursorx=xoffset;
  width=0;

  closestdist=99;
  querypos=point;

  for (strcur=buf;*strcur;++strcur) 

    if (*strcur<forty) 
      
      objx = cursorx += width, width = *strcur-34;
      
    else {

      op=*strcur/32&3; arg=*strcur++&31;
      objx+=*strcur/8&3; objy=*strcur&7;
      objpos=makevec(objx,objy,0); objdir=makevec(arg/8,(2*op-3)*(arg&7)+1e-4,0);

      if (op<3) 

        updatequery(addscale(objpos,objdir,clamp(dot(addscale(point,objpos,-1),objdir)/dot(objdir,objdir))),blue);

      else {

        real halfpi=M_PI*half, angle=atan2(point.yy-objy, point.xx-objx)/halfpi, lower=(arg/4&3)-2, range=(arg&3)+1, upper=lower+range;
        angle = angle>upper+half*range ? lower : angle > upper ? upper : angle<lower-half*range ? upper : angle<lower ? lower : angle;

        objpos.xx += cos(angle*halfpi);
        objpos.yy += sin(angle*halfpi);

        updatequery(objpos,blue);
        
      } 

    }


  normal=makevec(0,1,0); updatequery(addscale(point,normal,-dot(point,normal)-.9),white);

  closestmat = closestmat.xx==1 && ((int)(point.xx+64)/8^(int)(point.zz+64)/8)&1 ?
    makevec(twothird,0,0) : closestmat;

  normal=addscale(querypos,closest,-1);

  return sqrt(closestdist)-.45;

}

vec normalize(vec a) { return addscale(zerovec, a, 1/sqrt(dot(a,a))); }

vec raytrace(vec origin, vec dir, real depth) {

  real along=0, lastdist=0, aoiter=0, aoval=1, whiteness=dir.zz*dir.zz;
  vec pixel, thisnormal;

  while (along<97)
    if ((lastdist=dquery(pixel = addscale(origin,dir,along+=lastdist)))*lastdist<.001) {
      
      pixel=closestmat;
      thisnormal=normalize(normal);
          
      origin=addscale(origin,dir,along);

      while (++aoiter<6)  

        aoval -= clamp(aoiter/5-dquery(addscale(origin,thisnormal,aoiter/5)))/pow(2,aoiter);

      pixel = addscale(zerovec,pixel,(clamp(dot(thisnormal,lightpos))*third+twothird)*aoval);
      
      pixel = depth?addscale(addscale(zerovec,pixel,twothird),raytrace(addscale(origin,thisnormal,.1),addscale(dir,thisnormal,-2*dot(dir,thisnormal)),depth-1),third):pixel;

      along=pow(clamp(dot(thisnormal,normalize(addscale(lightpos,dir,-1)))),forty);

      return addscale(addscale(zerovec,pixel,1-along),white,along);

    }
  
  return makevec(whiteness,whiteness,1);

}

int main(int xcoord, char** argv) {

  for (strcur = xcoord>1 ? aamask=0, argv[1] : data+296; *strcur; ++strcur)
    for (op=arg=0, datacur=data; (curchar=*datacur)&&!(op&&*datacur<forty); ++datacur) 
      op ? (*bufcur++=curchar, *bufcur++=*++datacur) : 
        ( curchar<forty ? (width=curchar,++arg) : 
          curchar==forty ? (arg=*++datacur) : arg,
          arg == (*strcur^aamask) ? xoffset-=34-(*bufcur++=width), op=1 : xoffset);

  aamask=xcoord<3;
  *bufcur = 0;
  xoffset = 10-xoffset*half;
  lightpos=normalize(makevec(-1,1,2));
  
  puts("P6 600 220 255");

  while (++ycoord<110) 

    for (xcoord=-300;xcoord<300;++xcoord) {
      
      vec p = zerovec;

      for (curchar=0; curchar<aamask*3+1; ++curchar)
          p = addscale(p, raytrace( makevec(-4.5,5,30),
                      normalize( addscale( makevec(10.25,-2,-25) ,
                            addscale( addscale(zerovec, normalize(makevec(5,0,2)), xcoord+curchar/2*half ), 
                               normalize(makevec(-2,73,0)),-ycoord-half*(curchar&1)), .034) ),
                      2*aamask ), 
                aamask?63:255 );

      putchar(p.xx);
      putchar(p.yy);
      putchar(p.zz);


    }

  return 0;

}
