
#include "stats.h"
#include <math.h>


/* returns the sums of all pixel values across all color channels.
	* useful in computing the score of a rectangle
	* PA3 function
	* @param channel is one of r, g, or b
	* @param ul is (x,y) of the upper left corner of the rectangle
	* @param lr is (x,y) of the lower right corner of the rectangle
*/
long stats::getSum(char channel, pair<int,int> ul, pair<int,int> lr){
    switch(channel){
		case 'r':
			return getSumHelper(sumRed, ul, lr);
		case 'g':
			return getSumHelper(sumGreen, ul, lr);
		case 'b':
			return getSumHelper(sumBlue, ul, lr);
	}
	return 0;
}

long stats::getSumHelper(vector<vector<long>> & channel, pair<int, int> ul, pair<int,int> lr){
	if(ul.first==0 && ul.second==0){
		return channel[lr.second][lr.first];
	}else if(ul.first>0 && ul.second==0){
		return channel[lr.second][lr.first]-channel[lr.second][ul.first-1];
	}else if(ul.first==0 && ul.second>0){
		return channel[lr.second][lr.first]-channel[ul.second-1][lr.first];
	}else{
		return channel[lr.second][lr.first]-channel[lr.second][ul.first-1]-channel[ul.second-1][lr.first]+channel[ul.second-1][ul.first-1];
	}

}



/* returns the sums of squares of all pixel values across all color channels.
	* useful in computing the score of a rectangle
	* PA3 function
	* @param channel is one of r, g, or b
	* @param ul is (x,y) of the upper left corner of the rectangle
	* @param lr is (x,y) of the lower right corner of the rectangle */
long stats::getSumSq(char channel, pair<int,int> ul, pair<int,int> lr){
    switch(channel){
		case 'r':
			return getSumHelper(sumsqRed, ul, lr);
		case 'g':
			return getSumHelper(sumsqGreen, ul, lr);
		case 'b':
			return getSumHelper(sumsqBlue, ul, lr);
	}
	return 0;
}

// initialize the private vectors so that, for each color,  entry
// (x,y) is the cumulative sum of the the color values from (0,0)
// to (x,y). Similarly, the sumSq vectors are the cumulative
// sum of squares from (0,0) to (x,y).
stats::stats(PNG & im){
	RGBAPixel* pixel;
	long r;
	long g;
	long b;

	for(unsigned int y = 0; y<im.height(); y++){
		vector<long> rowRed;
		vector<long> rowGreen;
		vector<long> rowBlue;
		vector<long> rowSqRed;
		vector<long> rowSqGreen;
		vector<long> rowSqBlue;

		for(unsigned int x = 0; x<im.width(); x++){
			pixel = im.getPixel(x,y);
			r = pixel->r;
			g = pixel->g;
			b = pixel->b;
			if(y==0 && x==0){
				rowRed.push_back(r);
				rowGreen.push_back(g);
				rowBlue.push_back(b);
				rowSqRed.push_back(pow(r,2));
				rowSqGreen.push_back(pow(g,2));
				rowSqBlue.push_back(pow(b,2));
		    }
			else if(y==0){
				rowRed.push_back(rowRed[x-1]+r);
				rowGreen.push_back(rowGreen[x-1]+g);
				rowBlue.push_back(rowBlue[x-1]+b);
				rowSqRed.push_back(rowSqRed[x-1]+pow(r,2));
				rowSqGreen.push_back(rowSqGreen[x-1]+pow(g,2));
				rowSqBlue.push_back(rowSqBlue[x-1]+pow(b,2));
			}
			else if(x==0){
				rowRed.push_back(sumRed[y-1][0]+r);
				rowGreen.push_back(sumGreen[y-1][0]+g);
				rowBlue.push_back(sumBlue[y-1][0]+b);
				rowSqRed.push_back(sumsqRed[y-1][0]+pow(r,2));
				rowSqGreen.push_back(sumsqGreen[y-1][0]+pow(g,2));
				rowSqBlue.push_back(sumsqBlue[y-1][0]+pow(b,2));
			}else{
				rowRed.push_back(sumRed[y-1][x]+ rowRed[x-1] - sumRed[y-1][x-1] + r);
				rowGreen.push_back(sumGreen[y-1][x]+ rowGreen[x-1] - sumGreen[y-1][x-1] + g);
				rowBlue.push_back(sumBlue[y-1][x]+ rowBlue[x-1] - sumBlue[y-1][x-1] + b);
				rowSqRed.push_back(sumsqRed[y-1][x]+ rowSqRed[x-1] - sumsqRed[y-1][x-1] + pow(r,2));
				rowSqGreen.push_back(sumsqGreen[y-1][x]+ rowSqGreen[x-1] - sumsqGreen[y-1][x-1] + pow(g,2));
				rowSqBlue.push_back(sumsqBlue[y-1][x]+ rowSqBlue[x-1] - sumsqBlue[y-1][x-1] + pow(b,2));
			}
		}
		sumRed.push_back(rowRed);
		sumGreen.push_back(rowGreen);
		sumBlue.push_back(rowBlue);
		sumsqRed.push_back(rowSqRed);
		sumsqGreen.push_back(rowSqGreen);
		sumsqBlue.push_back(rowSqBlue);
	}
}

// given a rectangle, compute its sum of squared deviations from
// mean, over all color channels. Will be used to make split when
// building tree.
/* @param ul is (x,y) of the upper left corner of the rectangle
* @param lr is (x,y) of the lower right corner of the rectangle */
long stats::getScore(pair<int,int> ul, pair<int,int> lr){
	//Red
	long rSd = getSumSq('r', ul, lr)-(pow(getSum('r', ul, lr), 2)/rectArea(ul, lr));
	long gSd = getSumSq('g', ul, lr)-(pow(getSum('g', ul, lr), 2)/rectArea(ul, lr));
	long bSd = getSumSq('b', ul, lr)-(pow(getSum('b', ul, lr), 2)/rectArea(ul, lr));
    return rSd+gSd+bSd;
}


// given a rectangle, return the average color value over the
// rectangle as a pixel.
/* Each color component of the pixel is the average value of that
* component over the rectangle.
* @param ul is (x,y) of the upper left corner of the rectangle
* @param lr is (x,y) of the lower right corner of the rectangle */

RGBAPixel stats::getAvg(pair<int,int> ul, pair<int,int> lr){

    return RGBAPixel(getSum('r', ul, lr)/rectArea(ul, lr), getSum('g', ul, lr)/rectArea(ul, lr), getSum('b', ul, lr)/rectArea(ul, lr));
}

// given a rectangle, return the number of pixels in the rectangle
/* @param ul is (x,y) of the upper left corner of the rectangle
* @param lr is (x,y) of the lower right corner of the rectangle */
long stats::rectArea(pair<int,int> ul, pair<int,int> lr){
	return abs((lr.first+1-ul.first)*(lr.second+1-ul.second));
}

//debug functions
 void stats::print_sum(){
   cout << "Print_sum" << endl;
   for(unsigned int y = 0; y< sumRed[y].size(); y++){ //change this line if you want to do whole picture
     cout << "Row:" + to_string(y) << endl;
     for(unsigned int x = 0; x<sumRed[y].size(); x++){
       cout << " " + to_string(sumRed[y][x]) + " "; //change this line if you want to do different colors
     }
     cout << endl;
   }
   cout << endl;
 }

 void stats::print_sumsq(){
   cout << "Print_sumsq" << endl;
   for(unsigned int y = 0; y< sumRed[y].size(); y++){ //change this line if you want to do whole picture
     cout << "Row:" + to_string(y) << endl;
     for(unsigned int x = 0; x<sumsqRed[y].size(); x++){
       cout << " " + to_string(sumsqRed[y][x]) + " "; //change this line if you want to do different colors
     }
     cout << endl;
   }
   cout << endl;
 }

 void stats::print_original(PNG & im){
   RGBAPixel* p;
   cout << "Print_original" << endl;
   for(unsigned int y = 0; y < im.height(); y++){ //change this line if you want to do whole picture
     cout << "Row: " + to_string(y) << endl;
     for(unsigned int x = 0; x < im.width(); x++){
       p = im.getPixel(x,y);
       long r = p->r; //change this line if you want to do different colors
       cout << " " + to_string(r) + " ";
     }
     cout << endl;
   }
   cout << endl;
 }
