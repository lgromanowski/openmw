#include "subview.hpp"

#include "view.hpp"

CSVDoc::SubView::SubView (const CSMWorld::UniversalId& id) : mUniversalId (id)
{
    /// \todo  add a button to the title bar that clones this sub view

    setWindowTitle (QString::fromUtf8 (mUniversalId.toString().c_str()));
    setAttribute(Qt::WA_DeleteOnClose);
}

CSMWorld::UniversalId CSVDoc::SubView::getUniversalId() const
{
    return mUniversalId;
}

void CSVDoc::SubView::setStatusBar (bool show) {}

void CSVDoc::SubView::useHint (const std::string& hint) {}

void CSVDoc::SubView::updateUserSetting (const QString &, const QStringList &)
{}

void CSVDoc::SubView::setUniversalId (const CSMWorld::UniversalId& id)
{
    mUniversalId = id;
    setWindowTitle (mUniversalId.toString().c_str());
}

void CSVDoc::SubView::closeEvent (QCloseEvent *event)
{
    // update title bars of view and subviews
    if(mParent)
        mParent->updateSubViewIndicies(this);
}
