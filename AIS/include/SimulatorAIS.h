#pragma once

#include "ui_SimulatorAIS.h"
#include "BaseNaviWidget.h"  
#include "Type123Simulator.h"
#include "Type5Simulator.h"
namespace Ui
{
class SimulatorAIS;
}

class SimulatorAIS : public BaseNaviWidget
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "com.example.BaseNaviWidget/1.0")
    Q_INTERFACES(BaseNaviWidget)
public:
    explicit SimulatorAIS(QWidget *parent = nullptr);
    ~SimulatorAIS();

    virtual QIcon icon() const override;
    virtual QString name() const override;
    virtual QString description() const override;

protected slots:
    virtual QStringList getNavigationData() override;
private:
    Ui::SimulatorAIS *ui;
    Type123Simulator * type123 = nullptr;
    Type5Simulator * type5 = nullptr;
};
