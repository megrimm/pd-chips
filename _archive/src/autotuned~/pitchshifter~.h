		// *****************
		// * Pitch Shifter *
		// *****************
		
		// TODO: Pre-filter with some kind of filter (maybe cheby2 or just svf)
		// TODO: Use cubic spline interpolation
		
		// IMPROVE QUALITY OF PITCH SHIFTER!
		// what is the glitch at "lAaAack"? probably pitch shifter
		
		//   Better snippet management
		//   Pre-filter
		//   Cubic spline interp
		// Pitch shifter (overlap-add, pitch synchronous)
		//   Note: pitch estimate is naturally N/2 samples old
		x->phasein = x->phasein + x->phinc;
		x->phaseout = x->phaseout + x->phinc*x->phincfact;
		
		//   If it happens that there are no snippets placed at the output, grab a new snippet!
		/*     if (x->cbonorm[((long int)x->cbord + (long int)(N/2*(1 - (float)1 / x->phincfact)))%N] < 0.2) { */
		/*       fprintf(stderr, "help!"); */
		/*       x->phasein = 1; */
		/*       x->phaseout = 1; */
		/*     } */
		
		//   When input phase resets, take a snippet from N/2 samples in the past
		if (x->phasein >= 1) {
			x->phasein = x->phasein - 1;
			ti2 = x->cbiwr - (long int)N/2;
			for (ti=-((long int)N)/2; ti<(long int)N/2; ti++) {
				x->frag[ti%N] = x->cbi[(ti + ti2)%N];
			}
		}
		
		//   When output phase resets, put a snippet N/2 samples in the future
		if (x->phaseout >= 1) {
			x->fragsize = x->fragsize*2;
			if (x->fragsize >= N) {
				x->fragsize = N;
			}
			x->phaseout = x->phaseout - 1;
			ti2 = x->cbord + N/2;
			ti3 = (long int)(((float)x->fragsize) / x->phincfact);
			for (ti=-ti3/2; ti<(ti3/2); ti++) {
				tf = x->hannwindow[(long int)N/2 + ti*(long int)N/ti3];
				x->cbo[(ti + ti2)%N] = x->cbo[(ti + ti2)%N] + x->frag[((int)(x->phincfact*ti))%N]*tf;
				x->cbonorm[(ti + ti2)%N] = x->cbonorm[(ti + ti2)%N] + tf;
			}
			x->fragsize = 0;
		}
		x->fragsize++;
		
		//   Get output signal from buffer
		tf = x->cbonorm[x->cbord];
		//   Normalize
		if (tf>0.5) {
			tf = (float)1/tf;
		}
		else {
			tf = 1;
		}
		tf = tf*x->cbo[x->cbord]; // read buffer
		tf = x->cbo[x->cbord];
		x->cbo[x->cbord] = 0; // erase for next cycle
		x->cbonorm[x->cbord] = 0;
		x->cbord++; // increment read pointer
		if (x->cbord >= N) {
			x->cbord = 0;
		}
		
		// *********************
		// * END Pitch Shifter *
		// *********************