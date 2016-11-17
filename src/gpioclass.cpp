#include "gpioclass.h"

#include <fstream>
#include <string>
#include <iostream>
#include <sstream>

using namespace std;

GPIOClass::GPIOClass()
{
    this->gpionum = "17"; //GPIO17 is default
}

GPIOClass::GPIOClass(string gnum)
{
    this->gpionum = gnum;  //Instatiate GPIOClass object for GPIO pin number "gnum"
}

int GPIOClass::export_gpio()
{
    string export_str = "/sys/class/gpio/export";
    ofstream exportgpio(export_str.c_str()); // Open "export" file. Convert C++ string to C string. Required for all Linux pathnames
//    if (exportgpio < 0){
//        cout << " OPERATION FAILED: Unable to export GPIO"<< this->gpionum <<" ."<< endl;
//        return -1;
//    }

    exportgpio << this->gpionum ; //write GPIO number to export
    exportgpio.close(); //close export file
    return 0;
}

int GPIOClass::unexport_gpio()
{
    string unexport_str = "/sys/class/gpio/unexport";
    ofstream unexportgpio(unexport_str.c_str()); //Open unexport file
//    if (unexportgpio < 0){h
//        cout << " OPERATION FAILED: Unable to unexport GPIO"<< this->gpionum <<" ."<< endl;
//        return -1;
//    }

    unexportgpio << this->gpionum ; //write GPIO number to unexport
    unexportgpio.close(); //close unexport file
    return 0;
}

int GPIOClass::setdir_gpio(string dir)
{

    string setdir_str ="/sys/class/gpio/gpio" + this->gpionum + "/direction";
    ofstream setdirgpio(setdir_str.c_str()); // open direction file for gpio
//        if (setdirgpio < 0){
//            cout << " OPERATION FAILED: Unable to set direction of GPIO"<< this->gpionum <<" ."<< endl;
//            return -1;
//        }

        setdirgpio << dir; //write direction to direction file
        setdirgpio.close(); // close direction file
        return 0;
}

int GPIOClass::setval_gpio(string val)
{

    string setval_str = "/sys/class/gpio/gpio" + this->gpionum + "/value";
    ofstream setvalgpio(setval_str.c_str()); // open value file for gpio
//        if (setvalgpio < 0){
//            cout << " OPERATION FAILED: Unable to set the value of GPIO"<< this->gpionum <<" ."<< endl;
//            return -1;
//        }

        setvalgpio << val ;//write value to value file
        setvalgpio.close();// close value file
        return 0;
}

int GPIOClass::getval_gpio(string& val){

    string getval_str = "/sys/class/gpio/gpio" + this->gpionum + "/value";
    ifstream getvalgpio(getval_str.c_str());// open value file for gpio
//    if (getvalgpio < 0){
//        cout << " OPERATION FAILED: Unable to get value of GPIO"<< this->gpionum <<" ."<< endl;
//        return -1;
//            }

    getvalgpio >> val ;  //read gpio value

    if(val != "0")
        val = "1";
    else
        val = "0";

    getvalgpio.close(); //close the value file
    return 0;
}

string GPIOClass::get_gpionum(){

return this->gpionum;

}


//Test main.cpp

//#include <iostream>
//#include <unistd.h>
//#include <errno.h>
//#include <stdio.h>
//#include <stdlib.h>
//#include "GPIOClass.h"

//using namespace std;

//int main (void)
//{

//    string inputstate;
//    GPIOClass* gpio4 = new GPIOClass("4"); //create new GPIO object to be attached to  GPIO4
//    GPIOClass* gpio17 = new GPIOClass("17"); //create new GPIO object to be attached to  GPIO17

//    gpio4->export_gpio(); //export GPIO4
//    gpio17->export_gpio(); //export GPIO17

//    cout << " GPIO pins exported" << endl;

//    gpio17->setdir_gpio("in"); //GPIO4 set to output
//    gpio4->setdir_gpio("out"); // GPIO17 set to input

//    cout << " Set GPIO pin directions" << endl;

//    while(1)
//    {
//        usleep(500000);  // wait for 0.5 seconds
//        gpio17->getval_gpio(inputstate); //read state of GPIO17 input pin
//        cout << "Current input pin state is " << inputstate  <<endl;
//        if(inputstate == "0") // if input pin is at state "0" i.e. button pressed
//        {
//            cout << "input pin state is "Pressed ".n Will check input pin state again in 20ms "<<endl;
//                usleep(20000);
//                    cout << "Checking again ....." << endl;
//                    gpio17->getval_gpio(inputstate); // checking again to ensure that state "0" is due to button press and not noise
//            if(inputstate == "0")
//            {
//                cout << "input pin state is definitely "Pressed". Turning LED ON" <<endl;
//                gpio4->setval_gpio("1"); // turn LED ON

//                cout << " Waiting until pin is unpressed....." << endl;
//                while (inputstate == "0"){
//                gpio17->getval_gpio(inputstate);
//                };
//                cout << "pin is unpressed" << endl;

//            }
//            else
//                cout << "input pin state is definitely "UnPressed". That was just noise." <<endl;

//        }
//        gpio4->setval_gpio("0");

//    }
//    cout << "Exiting....." << endl;
//    return 0;
//}
