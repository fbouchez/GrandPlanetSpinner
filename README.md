# GrandPlanetSpinner

Credits go to illusionmanager for creating the Grand Planet Spinner

https://www.instructables.com/Grand-Planet-Spinner/

On this repository I host some of the original files, as well as modified 
versions that I used when recreating the Grand Planet Spinner for myself.



## Repository organization

* `original/` contains the original files by illusionmanager: `.png` files for 
  laser cutting and `.ino` file for the Arduino part (programming).

* `laser/` contains converted `svg` files for laser cutting based on the 
  original `.png` files. See updated instructions below.

* `src/` contains the Arduino code to run the Grand Planet Spinner.
  It currently is only a copy of the original code, but with the owner 
permission I plan to update the code as follows:
- use of the UART interface of the TMC2209
- use of the SpeedyStepper Arduino library
- programming of the LED to planets "color" based on
  https://www.shutterstock.com/fr/blog/le-code-couleur-de-univers





## Update on laser cutting and engraving

The `main.svg` file for Cutting / Engraving with a laser cutter is based on the
work by illusionmanager, with reduced manutention in mind.
Only 2 passes are required with a laser cutter than can handle engraving and 
cutting in the same pass, which reduces the risk of alignment errors and 
manipulation mistakes with the original 4 passes. It also requires flipping 
a rectangular wood part only once instead of twice.

The file contains 3 layers, two of which are used in actual cutting/engraving.
You may need to export these layers into separate `.svg` files if your laser 
cutter software does not handle svg layers.

### Legend:
- black shapes: engrave
- red lines: cut
- fuchsia lines: do not cut or engrave, just here for calibration/visual 
  verification


### How to use:
1. using layer `10-planets-back-engrave-cut-square`:
   engrave the back side of the planets and cut the square around them
2. flip the cut square in the machine, making sure not to move anything around
3. using layer `20-engrave-cut-all`:
   engrave the front side of the planets as well as all other engravings, then
   proceed with the cutting of all parts, incl. the planets themselves.
4. cut the spacers using the `spacers.svg` file

Note: the layer `check-planets-mirror` is not used for cutting/engraving, it is 
just there to visually check that the front and back of planets are aligned.
(I had vertical alignment problems with the original files converted from `.png`.)

Second note: the sun (inside the B part) did not get well from the 
vectorization process.  TODO: update the graphics of the sun.


### Graphics

Original graphics of planets and zodiac disk are of unknown attribution.

The updated vectorial sun is from
[freepik.com](href="https://www.freepik.com/free-vector/hand-drawn-sun-outline-illustration_43582961.htm#query=sun%20svg&position=16&from_view=keyword&track=ais">Freepik)



