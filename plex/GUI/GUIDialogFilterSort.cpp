//
//  GUIDialogFilterSort.cpp
//  Plex
//
//  Created by Tobias Hieta <tobias@plexapp.com> on 2012-11-26.
//  Copyright 2012 Plex Inc. All rights reserved.
//

#include "GUIDialogFilterSort.h"
#include "plex/PlexTypes.h"
#include "guilib/GUIControlGroupList.h"
#include "guilib/GUILabelControl.h"
#include "GUIWindowManager.h"
#include "LocalizeStrings.h"

///////////////////////////////////////////////////////////////////////////////////////////////////
CGUIDialogFilterSort::CGUIDialogFilterSort()
  : CGUIDialog(WINDOW_DIALOG_FILTER_SORT, "DialogFilters.xml")
{
  m_loadType = LOAD_ON_GUI_INIT;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void CGUIDialogFilterSort::SetFilter(CPlexSecondaryFilterPtr filter, int filterButtonId)
{
  m_filter = filter;
  m_filterButtonId = filterButtonId;

  if (!m_filter->hasValues())
  {
    /* We should always have values at this point! */
    return;
  }

  CGUIControlGroupList* list = (CGUIControlGroupList*)GetControl(FILTER_SUBLIST);
  if (!list)
    return;

  list->ClearAll();

  CGUIRadioButtonControl* radioButton = (CGUIRadioButtonControl*)GetControl(FILTER_SUBLIST_RADIO_BUTTON);
  if (!radioButton)
    return;
  radioButton->SetVisible(false);

  CGUIButtonControl* clearFilters = (CGUIRadioButtonControl*)GetControl(FILTER_SUBLIST_BUTTON);
  if (!clearFilters)
    return;
  clearFilters->SetVisible(false);

  CGUILabelControl* headerLabel = (CGUILabelControl*)GetControl(FILTER_SUBLIST_LABEL);
  if (headerLabel)
    headerLabel->SetLabel(m_filter->getFilterTitle());

  PlexStringPairVector values = m_filter->getFilterValues();

  int id = FILTER_SUBLIST_BUTTONS_START;

  m_clearFilters = new CGUIButtonControl(*clearFilters);
  m_clearFilters->SetLabel(g_localizeStrings.Get(44032));
  m_clearFilters->SetVisible(false);
  m_clearFilters->AllocResources();
  m_clearFilters->SetID(FILTER_SUBLIST_CLEAR_FILTERS);
  list->AddControl(m_clearFilters);

  BOOST_FOREACH(PlexStringPair p, values)
  {
    CGUIRadioButtonControl* sublistItem = new CGUIRadioButtonControl(*radioButton);
    sublistItem->SetLabel(p.second);
    sublistItem->SetVisible(true);
    sublistItem->AllocResources();
    sublistItem->SetID(id);
    sublistItem->SetSelected(m_filter->isSelected(p.first));

    filterControl fc;
    fc.first = p;
    fc.second = sublistItem;
    m_filterMap[id] = fc;

    list->AddControl(sublistItem);

    id++;
  }

  m_clearFilters->SetVisible(m_filter->isSelected());
}

///////////////////////////////////////////////////////////////////////////////////////////////////
bool CGUIDialogFilterSort::OnAction(const CAction &action)
{
  if (action.GetID() == ACTION_CLEAR_FILTERS)
  {
    m_filter->clearFilters();

    for (int i = FILTER_SUBLIST_BUTTONS_START; i < 0; i++)
    {
      CGUIRadioButtonControl* button = (CGUIRadioButtonControl*)GetControl(i);
      if (!button)
        break;

      button->SetSelected(false);
    }

    m_clearFilters->SetVisible(false);

    CGUIMessage msg(GUI_MSG_FILTER_SELECTED, WINDOW_DIALOG_FILTER_SORT, 0, m_filterButtonId, 0);
    msg.SetStringParam(m_filter->getFilterKey());
    g_windowManager.SendThreadMessage(msg, g_windowManager.GetActiveWindow());

    SetInvalid();

    return true;
  }

  return CGUIDialog::OnAction(action);
}

///////////////////////////////////////////////////////////////////////////////////////////////////
bool CGUIDialogFilterSort::OnMessage(CGUIMessage &message)
{
  switch (message.GetMessage())
  {
    case GUI_MSG_CLICKED:
    {
      int senderId = message.GetSenderId();
      if (m_filterMap.find(senderId) != m_filterMap.end())
      {
        CGUIRadioButtonControl *filterCtrl = m_filterMap[senderId].second;
        PlexStringPair filterKv = m_filterMap[senderId].first;
        m_filter->setSelected(filterKv.first, filterCtrl->IsSelected());

        m_clearFilters->SetVisible(m_filter->isSelected());

        CGUIMessage msg(GUI_MSG_FILTER_SELECTED, WINDOW_DIALOG_FILTER_SORT, 0, m_filterButtonId, 0);
        msg.SetStringParam(m_filter->getFilterKey());
        g_windowManager.SendThreadMessage(msg, g_windowManager.GetActiveWindow());

        SetInvalid();
        return true;
      }
    }

  }
  return CGUIDialog::OnMessage(message);
}
