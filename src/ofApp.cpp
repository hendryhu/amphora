// Refer to the README.md in the example's root folder for more information on usage

#include "ofApp.h"


//--------------------------------------------------------------
void ofApp::setup()
{
    counter = 0;

    ofBackground(0); // black background
    ofEnableDepthTest(); // enable visibility testing

    ofSetWindowTitle("Amphora"); // because that's what this is demo'ing

    mymesh = createAmphoraMesh(240, 100, 20, 20); // creates the amphora, go figure

    ofSetFrameRate(60); // caps the framerate at 60fps.

    cout << "Vertices: " << mymesh.getNumVertices() << endl;
    cout << "Indices: " << mymesh.getNumIndices() << endl; // some debugging to check that there is geometry
}

//--------------------------------------------------------------
void ofApp::update()
{
    // update event slowly increments the counter variable
    counter = counter + 0.033f;

    // set the duration of the square path, and calculate the position given the counter (time)
    float duration = 4.0f;
    squarePos = parametricSquare(counter, duration) * 200.0f;
}

//--------------------------------------------------------------
void ofApp::draw()
{
    ofBackground(0);
    cam.begin(); // using ofEasyCam
    ofPushMatrix();
    ofTranslate(squarePos.x, squarePos.y, squarePos.z);  // move the mesh (and the normals) to the parametric square position
    mymesh.draw(); // try .drawWireFrame() to see the mesh structure
    // mymesh.drawWireframe();

    // draw the normals
    auto n = mymesh.getNormals();
    auto v = mymesh.getVertices();
    float normalLength = 10;
    ofSetColor(255, 255, 255, 80);
    for (int i = 0; i < n.size(); i++)
    {
        ofDrawLine(v[i], v[i] + n[i] * normalLength);
    }
    
    ofPopMatrix();    
    cam.end();
}


//--------------------------------------------------------------
void ofApp::keyPressed(int key)
{
}

//--------------------------------------------------------------
void ofApp::keyReleased(int key)
{
}

//--------------------------------------------------------------
void ofApp::mouseMoved(int x, int y)
{
}

//--------------------------------------------------------------
void ofApp::mouseDragged(int x, int y, int button)
{
}

//--------------------------------------------------------------
void ofApp::mousePressed(int x, int y, int button)
{
}

//--------------------------------------------------------------
void ofApp::mouseReleased(int x, int y, int button)
{
}

//--------------------------------------------------------------
void ofApp::mouseEntered(int x, int y)
{
}

//--------------------------------------------------------------
void ofApp::mouseExited(int x, int y)
{
}

//--------------------------------------------------------------
void ofApp::windowResized(int w, int h)
{
}

//--------------------------------------------------------------
void ofApp::gotMessage(ofMessage msg)
{
}

//--------------------------------------------------------------
void ofApp::dragEvent(ofDragInfo dragInfo)
{
}

ofMesh ofApp::createAmphoraMesh(float H, float r, int majorRes, int minorRes)
{
    ofMesh mesh;
    mesh.setMode(OF_PRIMITIVE_TRIANGLES); // bunch of individual triangles

    // defining the thickness of the amphora by having height-radius pairs (X, Y)
    // both go from 0 to 1, defining at X height, the radius is Y thick
    std::vector<std::pair<float, float>> hrPairs = {
        {0.0, 0.5},
        {0.1, 0.4},
        {0.5, 0.8},
        {0.7, 0.3},
        {1.0, 0.5}
    };

    // draw each ring
    for (int i = 0; i < majorRes; i++)
    {
        float t = static_cast<float>(i) / majorRes; // linear interpolation from 0 to 1 for the height

        // draw each vertex in the ring
        for (int j = 0; j < minorRes; j++)
        {
            // position on the circle
            float phi = TWO_PI * j / minorRes;

            // interpolate the radius at this height
            float radius_at_t = 0;
            for (int k = 0; k < hrPairs.size() - 1; k++)
            {
                if (hrPairs[k].first <= t && t <= hrPairs[k + 1].first)
                {
                    float increment = (t - hrPairs[k].first) / (hrPairs[k + 1].first - hrPairs[k].first);
                    increment = increment * increment * (3 - 2 * increment); // smoothstep formula
                    radius_at_t = ofLerp(hrPairs[k].second, hrPairs[k + 1].second, increment);
                }
            }

            // calculate the vertex position
            glm::vec3 v = glm::vec3(
                radius_at_t * r * cos(phi),
                (t - 0.5) * H, // center the amphora at y=0
                radius_at_t * r * sin(phi)
            );

            ofColor c = ofColor(255 * (i + 0.001) / majorRes, 255 * (j + 0.001) / minorRes, 255 * 0.5);

            mesh.addVertex(v);
            mesh.addColor(c); // applies to most recent vertex
        }
    }

    // create the bottom vector to close the amphora
    glm::vec3 bottomCenter = glm::vec3(0, -0.5 * H, 0);
    mesh.addVertex(bottomCenter);
    mesh.addColor(ofColor(255, 0, 0));

    // create triangles from the middle to every point on the bottom ring
    for (int j = 0; j < minorRes; j++)
    {
        int jNext = (j + 1) % minorRes;

        int id0 = mesh.getNumVertices() - 1; // center vertex
        int id1 = j;
        int id2 = jNext;

        mesh.addIndex(id0);
        mesh.addIndex(id1);
        mesh.addIndex(id2);
    }

    // create meshes out of the vertices
    for (int i = 0; i < majorRes - 1; i++)
    {
        for (int j = 0; j < minorRes; j++)
        {
            int iNext = i + 1; // don't create a mesh from the top to the bottom
            int jNext = (j + 1) % minorRes;

            int id0 = i * minorRes + j;
            int id1 = i * minorRes + jNext;
            int id2 = iNext * minorRes + jNext;
            int id3 = iNext * minorRes + j;

            mesh.addIndex(id0);
            mesh.addIndex(id1);
            mesh.addIndex(id2);

            mesh.addIndex(id0);
            mesh.addIndex(id2);
            mesh.addIndex(id3);
        }
    }

    // calculate the normals
    std::vector<glm::vec3> normals(mesh.getNumVertices(), glm::vec3(0, 0, 0));

    for (int i = 0; i < majorRes; i++)
    {
        for (int j = 0; j < minorRes; j++)
        {
            int currentIndex = i * minorRes + j;

            // Tangent 1 (horizontally along the ring, built from what is left and right of the current vertex)
            int prevJ = (j - 1 + minorRes) % minorRes;
            int nextJ = (j + 1) % minorRes;

            int prevJ_Index = i * minorRes + prevJ;
            int nextJ_Index = i * minorRes + nextJ;

            glm::vec3 v_prev_j = mesh.getVertex(prevJ_Index);
            glm::vec3 v_next_j = mesh.getVertex(nextJ_Index);

            glm::vec3 tangent_ring = v_next_j - v_prev_j;


            // Tangent 2 (vertically along the profile, built from what is above and below the current vertex)
            glm::vec3 tangent_profile;

            if (i == 0)
            {
                // no ring below, just use this one and the one above
                int above_Index = (i + 1) * minorRes + j;
                glm::vec3 v_current = mesh.getVertex(currentIndex);
                glm::vec3 v_above = mesh.getVertex(above_Index);
                tangent_profile = v_above - v_current;
            }
            else if (i == majorRes - 1)
            {
                // no ring above, just use this one and the one below
                int below_Index = (i - 1) * minorRes + j;
                glm::vec3 v_current = mesh.getVertex(currentIndex);
                glm::vec3 v_below = mesh.getVertex(below_Index);
                tangent_profile = v_current - v_below;
            }
            else
            {
                // in the middle, use the one above and below
                int below_Index = (i - 1) * minorRes + j;
                int above_Index = (i + 1) * minorRes + j;
                glm::vec3 v_below = mesh.getVertex(below_Index);
                glm::vec3 v_above = mesh.getVertex(above_Index);
                tangent_profile = v_above - v_below;
            }

            // the normal is then the cross product of the profile and ring tangents
            glm::vec3 normal = glm::cross(tangent_profile, tangent_ring);
            normals[currentIndex] = glm::normalize(normal);
        }
    }

    // the bottom center normal is straight down
    normals[mesh.getNumVertices() - 1] = glm::vec3(0, -1, 0);

    mesh.addNormals(normals);

    return mesh;
}

// calculate the position on a square path
glm::vec3 ofApp::parametricSquare(float current_time, float duration)
{
    const float t = fmod(current_time, duration) / duration; // normalized time [0,1]

    // helper variables
    const float A = fmod(4.0f*t, 4.0f);
    const float B = fmod(4.0f*t - 1.0f, 4.0f);  // phase shift by 1 

    // parametric equations for x and z
    const float x = max(0.0f, A) - max(0.0f, A - 1.0f) - max(0.0f, A - 2.0f) + max(0.0f, A - 3.0f) - 0.5f;
    const float z = max(0.0f, B) - max(0.0f, B - 1.0f) - max(0.0f, B - 2.0f) + max(0.0f, B - 3.0f) - 0.5f;
    
    const auto p = glm::vec3(x, 0, z);
    return p;
}
