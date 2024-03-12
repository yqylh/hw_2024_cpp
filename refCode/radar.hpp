#ifndef __RADAR_CPP__
#define __RADAR_CPP__

#include <iostream>
#include <cmath>
#include <vector>


#ifdef DEBUG_RADAR
#include <fstream>
std::ofstream radarOutput("radarOutput.txt");
#define LOGGER_RADAR(x) radarOutput << x << std::endl;
#else
#define LOGGER_RADAR(x)
#endif

#define EQUAL(x, y, eps) (fabs(x - y) < eps)
#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif
class TVector {
public:
    double x, y;
    TVector(const double &x_, const double &y_) {
        x = x_;
        y = y_;
    }
    TVector() : x(0), y(0) {}

    TVector operator + (const TVector& other) const { return TVector(x + other.x, y + other.y); }
    TVector operator - (const TVector& other) const { 
        /*
        LOGGER_RADAR((x == 23.2) << " " << (y == 37.75) << " " << (other.x == 23.1957) << " " << (other.y == 37.812));
        LOGGER_RADAR("x=" << x << " y=" << y << " ox=" << other.x << " oy=" << other.y);
        LOGGER_RADAR("x-ox=" << x - other.x << " y-oy=" << y - other.y);
        LOGGER_RADAR("x+ox=" << x + other.x << " y+oy=" << y + other.y);
        double xtmp = 23.2, oxtmp = 23.1957, ytmp = 37.75, oytmp = 37.812;
        LOGGER_RADAR("xtmp=" << xtmp << " ytmp=" << ytmp << " oxtmp=" << oxtmp << " oytmp=" << oytmp);
        LOGGER_RADAR("xtmp-oxtmp=" << xtmp - oxtmp << " ytmp-oytmp=" << ytmp - oytmp);
        LOGGER_RADAR("xtmp+oxtmp=" << xtmp + oxtmp << " ytmp+oytmp=" << ytmp + oytmp);
        */
        return TVector(x - other.x, y - other.y); 
    }
    TVector operator * (double scalar) const { return TVector(x * scalar, y * scalar); }
    TVector operator / (double scalar) const { return TVector(x / scalar, y / scalar); }
    double operator * (const TVector& other) const { return x * other.x + y * other.y; }
    double operator ^ (const TVector& other) const { return x * other.y - y * other.x; }
    bool operator == (const TVector& other) const { return x == other.x && y == other.y; }
    void operator = (const TVector& other) { x = other.x; y = other.y; }
    
    TVector& operator+=(const TVector& other) {
        x += other.x;
        y += other.y;
        return *this;
    }

    TVector& operator-=(const TVector& other) {
        x -= other.x;
        y -= other.y;
        return *this;
    }

    TVector& operator*=(double scalar) {
        x *= scalar;
        y *= scalar;
        return *this;
    }

    TVector& operator/=(double scalar) {
        x /= scalar;
        y /= scalar;
        return *this;
    }

    double modulo() const { return sqrt(x * x + y * y); }
    double modulo2() const { return x * x + y * y; }
    TVector unitize() const { return *this / modulo(); }
    TVector rotate(double angle) const {
        double c = cos(angle);
        double s = sin(angle);
        return TVector(x * c - y * s, x * s + y * c);
    }
    TVector rotate90() const { return TVector(-y, x); }
    TVector rotate270() const { return TVector(y, -x); }
};

class Radar {
public:
    Radar() {}
    Radar(const double &initX, const double initY, const double &initDirection, const std::vector<double> &laser) {
        self_initDirection = initDirection;
        self_laser = laser;
        self_initX = initX;
        self_initY = initY;
        getContactCoor();
    }

    std::vector<double> self_laser;
    double self_initDirection;
    double self_initX, self_initY;
    std::vector<TVector> self_contactCoor;

    static inline TVector polarCoorToVector(const double &r, const double &theta) {
        return TVector(r * cos(theta), r * sin(theta));
    }

    void getContactCoor() {
        self_contactCoor.clear();

        TVector initPosition = TVector(self_initX, self_initY);
        double initLaserAngle = self_initDirection;
        for (int nowLaserId = 0; nowLaserId < 360; nowLaserId++) {
            double nowLaserDis = self_laser[nowLaserId];
            double nowLaserAngle = initLaserAngle + (double)nowLaserId * M_PI / 180.0;

            TVector nowLaserVector = polarCoorToVector(nowLaserDis, nowLaserAngle);

            auto nowPosition = initPosition + nowLaserVector;

            self_contactCoor.push_back(nowPosition);
        }
    }

    void searchRobot(std::vector<std::vector<double>> &robotPositionInput, std::vector<int> &robotCarryInput) {
        double robotRadius = 0.45;
        double radiusRobotCarry = 0.53;

        std::vector<std::vector<TVector>> robotPoint;
        std::vector<TVector> robotPosition;
        std::vector<int> robotCarry;

        std::vector<TVector> contactPointBuffer;
        TVector robotCenterBuffer;
        std::vector<TVector> contactCarryPointBuffer;
        TVector robotCarryCenterBuffer;
        for (int nowLaserId = 0; nowLaserId < 360; nowLaserId++) {
            int nextLaserId = nowLaserId + 1;
            int lastLaserId = nowLaserId - 1;
            if (nextLaserId == 360) nextLaserId = 0;
            if (lastLaserId == -1) lastLaserId = 359;

            TVector lastContactCoor = self_contactCoor[lastLaserId];
            TVector nowContactCoor = self_contactCoor[nowLaserId];
            TVector nextContactCoor = self_contactCoor[nextLaserId];

            auto dirA = (nowContactCoor - lastContactCoor).unitize();
            auto dirB = (nextContactCoor - nowContactCoor).unitize();
            if (EQUAL(dirA.x, dirB.x, 1e-3) && EQUAL(dirA.y, dirB.y, 1e-3)) continue;

            TVector midLastAndNext = (lastContactCoor + nextContactCoor) / 2;
            TVector normalLastAndNext = (nextContactCoor - lastContactCoor).unitize().rotate270();

            // LOGGER_RADAR("lid=" << nowLaserId << " " << nowContactCoor.x << " " << nowContactCoor.y)
            double halfLastToNextDis2 = (nextContactCoor - lastContactCoor).modulo2() / 4.0;
            if (halfLastToNextDis2 > radiusRobotCarry * radiusRobotCarry) continue;
            

            double midPointToCenterDisRobotCarry = sqrt(radiusRobotCarry * radiusRobotCarry - halfLastToNextDis2);
            TVector robotCarryCenter = midLastAndNext + (normalLastAndNext * midPointToCenterDisRobotCarry);
            TVector nowToRobotCarryCenter = robotCarryCenter - nowContactCoor;
            // LOGGER_RADAR("\t" << (robotCarryCenter - lastContactCoor).modulo() << " " << (robotCarryCenter - nextContactCoor).modulo());
            // LOGGER_RADAR("\t" << nowToRobotCarryCenter.modulo() << " " << robotCarryCenter.x << " " << robotCarryCenter.y);
            if (EQUAL(nowToRobotCarryCenter.modulo2(), radiusRobotCarry * radiusRobotCarry, 1e-6)) {
                robotCarryCenterBuffer = robotCarryCenter;
                if (contactCarryPointBuffer.size() == 0) {
                    contactCarryPointBuffer.push_back(lastContactCoor);
                }
                contactCarryPointBuffer.push_back(nowContactCoor);
                /*
                LOGGER_RADAR("Robot carry found at " << robotCarryCenter.x << " " << robotCarryCenter.y);
                LOGGER_RADAR("\t" << dirA.x << " " << dirA.y);
                LOGGER_RADAR("\t" << dirB.x << " " << dirB.y);
                LOGGER_RADAR("\t" << EQUAL(dirA.x, dirB.x, 1e-3) << " " << EQUAL(dirA.y, dirB.y, 1e-3));
                LOGGER_RADAR("\t" << nowContactCoor.x << " " << nowContactCoor.y);
                LOGGER_RADAR("\t" << nowToRobotCarryCenter.modulo() << "\n\t" << robotCarryCenter.x << " " << robotCarryCenter.y);
                */
            } else {
                if (contactCarryPointBuffer.size() > 0) {
                    contactCarryPointBuffer.push_back(nowContactCoor);
                    if (contactCarryPointBuffer.size() >= 3) {
                        // robotPoint.push_back(contactCarryPointBuffer);
                        // robotPosition.push_back(robotCarryCenterBuffer);
                        // robotCarry.push_back(1);
                        robotPositionInput.push_back({robotCarryCenterBuffer.x, robotCarryCenterBuffer.y});
                        robotCarryInput.push_back(1);
                        // LOGGER_RADAR("Robot carry position found at " << robotCarryCenterBuffer.x << " " << robotCarryCenterBuffer.y);
                    }
                    contactCarryPointBuffer.clear();
                }
            }

            if (halfLastToNextDis2 > radiusRobotCarry * robotRadius) continue;

            double midPointToCenterDis = sqrt(robotRadius * robotRadius - halfLastToNextDis2);
            TVector robotCenter = midLastAndNext + (normalLastAndNext * midPointToCenterDis);
            TVector nowToRobotCenter = robotCenter - nowContactCoor;
            
            /*
            LOGGER_RADAR("\t" << (nowContactCoor - lastContactCoor).x << (nowContactCoor - lastContactCoor).y)
            LOGGER_RADAR("\t" << (nowContactCoor - lastContactCoor).modulo2())
            LOGGER_RADAR("\t" << (nowContactCoor - lastContactCoor).modulo2() / 4.0)
            LOGGER_RADAR("\t" << halfLastToNowDis2)
            LOGGER_RADAR("\t" << midLastAndNow.x << " " << midLastAndNow.y)
            LOGGER_RADAR("\t" << normalLastAndNow.x << " " << normalLastAndNow.y)
            LOGGER_RADAR("\t" << midPointToCenterDis)
            LOGGER_RADAR("\t" << robotCenter.x << " " << robotCenter.y)
            LOGGER_RADAR("\t" << nextToRobotCenter.x << " " << nextToRobotCenter.y)
            */
            // LOGGER_RADAR("\t" << nowToRobotCenter.modulo() << "\n\t" << robotCenter.x << " " << robotCenter.y);
            // LOGGER_RADAR("\t" << midPointToCenterDis << " " << halfLastToNowDis2)
            
            // LOGGER_RADAR("\t" << nowToRobotCenter.modulo());
            if (EQUAL(nowToRobotCenter.modulo2(), robotRadius * robotRadius, 1e-6)) {
                robotCenterBuffer = robotCenter;
                if (contactPointBuffer.size() == 0) {
                    contactPointBuffer.push_back(lastContactCoor);
                }
                contactPointBuffer.push_back(nowContactCoor);
                // LOGGER_RADAR("Robot found at " << robotCenter.x << " " << robotCenter.y);
                // LOGGER_RADAR("\t" << nowToRobotCenter.modulo() << "\n\t" << robotCenter.x << " " << robotCenter.y);
            } else {
                if (contactPointBuffer.size() > 0) {
                    contactPointBuffer.push_back(nowContactCoor);
                    if (contactPointBuffer.size() >= 3) {
                        // robotPoint.push_back(contactPointBuffer);
                        // robotPosition.push_back(robotCenterBuffer);
                        // robotCarry.push_back(0);
                        robotPositionInput.push_back({robotCenterBuffer.x, robotCenterBuffer.y});
                        robotCarryInput.push_back(0);
                        // LOGGER_RADAR("Robot position found at " << robotCenterBuffer.x << " " << robotCenterBuffer.y);
                    }
                    contactPointBuffer.clear();
                }
            }


        }
    }
};

#endif