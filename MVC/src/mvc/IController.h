// IController.h - MVC Controller interface
#ifndef SOLID_MVC_ICONTROLLER_H
#define SOLID_MVC_ICONTROLLER_H

class IController {
public:
    virtual ~IController() {}

    // Start controller main loop / processing
    virtual void start() = 0;

    // Stop controller
    virtual void stop() = 0;
};

#endif // SOLID_MVC_ICONTROLLER_H
