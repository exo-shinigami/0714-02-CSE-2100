// ControllerImpl.h — IController implementation (depends only on MVC interfaces + engine types).
#ifndef SOLID_MVC_CONTROLLERIMPL_H
#define SOLID_MVC_CONTROLLERIMPL_H

#include "../IController.h"
#include "../IModel.h"
#include "../IView.h"
#include "../InputEvent.h"
#include <vector>
#include "types_definitions.h"

class IEngineMovePolicy;

class ControllerImpl : public IController {
public:
    ControllerImpl(IModel* model, IView* view, SearchInfo* info, const IEvaluator& eval,
                     const IEngineMovePolicy& movePolicy);
    ~ControllerImpl();

    void start() override;
    void stop() override;

private:
    void handleInputEvent(const InputEvent& event);

    IModel* model_;
    IView* view_;
    SearchInfo* info_;
    const IEvaluator* evaluator_;
    const IEngineMovePolicy* movePolicy_;
    bool running_;

    int selectedSquare_;
    int promotionFrom_;
    int promotionTo_;
    bool promotionPending_;
    std::vector<int> possibleMoves_;
};

#endif // SOLID_MVC_CONTROLLERIMPL_H
