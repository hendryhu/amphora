// Refer to the README.md in the example's root folder for more information on usage

#pragma once

#include "ofMain.h"

class ofApp : public ofBaseApp{
	
	public:
		
		void setup();
		void update();
		void draw();
		
		void keyPressed(int key);
		void keyReleased(int key);
		void mouseMoved(int x, int y );
		void mouseDragged(int x, int y, int button);
		void mousePressed(int x, int y, int button);
		void mouseReleased(int x, int y, int button);
		void mouseEntered(int x, int y);
		void mouseExited(int x, int y);
		void windowResized(int w, int h);
		void dragEvent(ofDragInfo dragInfo);
		void gotMessage(ofMessage msg);		
		ofMesh createAmphoraMesh(float r1, float r2, int res1, int res2);
		static glm::vec3 parametricSquare(float t, float duration);

		// variables for this demo
		float counter;
		glm::vec3 squarePos;
		ofMesh mymesh;
		ofEasyCam cam; // sooo easy
};
