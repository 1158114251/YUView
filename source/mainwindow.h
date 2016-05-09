/*  YUView - YUV player with advanced analytics toolset
*   Copyright (C) 2015  Institut für Nachrichtentechnik
*                       RWTH Aachen University, GERMANY
*
*   YUView is free software; you can redistribute it and/or modify
*   it under the terms of the GNU General Public License as published by
*   the Free Software Foundation; either version 2 of the License, or
*   (at your option) any later version.
*
*   YUView is distributed in the hope that it will be useful,
*   but WITHOUT ANY WARRANTY; without even the implied warranty of
*   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
*   GNU General Public License for more details.
*
*   You should have received a copy of the GNU General Public License
*   along with YUView.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QListWidget>
#include <QList>
#include <QAction>
#include <QMenu>
#include <QMenuBar>
#include <QString>
#include <QMessageBox>
#include <QSettings>
#include <QTreeWidget>
#include <QMouseEvent>
#include <QTreeWidgetItem>
#include <QDesktopWidget>
#include <QKeyEvent>

#include "settingswindow.h"
#include "playlistTreeWidget.h"
#include "playlistItem.h"
#include "videoCache.h"

typedef enum {
  WindowModeSingle,
  WindowModeSeparate
} WindowMode;

namespace Ui {
  class MainWindow;
}

class SeparateWindow : public QMainWindow
{
  Q_OBJECT

public:
  SeparateWindow() : QMainWindow() 
  {
    // Create a new splitViewWidget and set it as center widget
    splitView = new splitViewWidget(this, true);
    setCentralWidget(splitView);
    splitView->setAttribute(Qt::WA_AcceptTouchEvents);

    connect(splitView, SIGNAL(signalToggleFullScreen()), this, SLOT(toggleFullscreen()));
  };
  splitViewWidget *splitView;
signals:
  // Signal that the user wants to go back to single window mode
  void signalSingleWindowMode();
  // Signals that originate from the keyboard shortcuts
  void signalNextFrame();
  void signalPreviousFrame();
  void signalPlayPauseToggle();
  void signalNextItem();
  void signalPreviousItem();
public slots:
  void toggleFullscreen()
  {
    QSettings settings;
    if (isFullScreen())
    {
      // Restore to normal
      showNormal();

      restoreState(settings.value("separateViewWindow/windowState").toByteArray());
      restoreGeometry(settings.value("separateViewWindow/geometry").toByteArray());
    }
    else
    {
      // Save current window layout
      settings.setValue("separateViewWindow/geometry", saveGeometry());
      settings.setValue("separateViewWindow/windowState", saveState());
      
      // Go full screen
      showFullScreen();
    }
  }
protected:
  void closeEvent(QCloseEvent *event) Q_DECL_OVERRIDE
  {
    // This window cannot be closed. Signal that we want to go to single window mode.
    // The main window will then hide this window.
    event->ignore();
    emit signalSingleWindowMode();
  }
  void keyPressEvent( QKeyEvent * event ) Q_DECL_OVERRIDE
  {
    int key = event->key();
    bool control = (event->modifiers() == Qt::ControlModifier);
    if (key == Qt::Key_F && control)
      toggleFullscreen();
    else if (key == Qt::Key_1 && control)
      emit signalSingleWindowMode();
    else if (key == Qt::Key_0 && control)
      splitView->resetViews();
    else if (key == Qt::Key_9 && control)
      splitView->zoomToFit();
    else if (key == Qt::Key_Plus && control)
      splitView->zoomIn();
    else if (key == Qt::Key_BracketRight && control)
      // This seems to be a bug in the Qt localization routine. On the german keyboard layout this key is returned
      // if Ctrl + is pressed. 
      splitView->zoomIn();
    else if (key == Qt::Key_Minus && control)
      splitView->zoomOut();
    else if (key == Qt::Key_Escape)
    {
      if (isFullScreen())
        toggleFullscreen();
    }
    else if (key == Qt::Key_Space)
      emit signalPlayPauseToggle();
    else if (key == Qt::Key_Right)
      emit signalNextFrame();
    else if (key == Qt::Key_Left)
      emit signalPreviousFrame();
    else if (key == Qt::Key_Down)
      emit signalNextItem();
    else if (key == Qt::Key_Up)
      emit signalPreviousItem();
    else
      QWidget::keyPressEvent(event);
    }
};

class MainWindow : public QMainWindow
{
  Q_OBJECT

public:
  explicit MainWindow(QWidget *parent = 0);
  ~MainWindow();

  bool eventFilter(QObject * target, QEvent * event);
  void closeEvent(QCloseEvent *event) Q_DECL_OVERRIDE;
  
private:
  PlaylistTreeWidget *p_playlistWidget;
  Ui::MainWindow *ui;

  QMessageBox *p_msg;
  bool p_ClearFrame;

  QMenu* fileMenu;
  QMenu* viewMenu;
  QMenu* playbackMenu;
  QMenu* helpMenu;

public:
  void loadFiles(QStringList files) { p_playlistWidget->loadFiles( files ); }
  void loadPlaylistFile(QString filePath);

  bool isPlaylistItemSelected() { return selectedPrimaryPlaylistItem() != NULL; }

public slots:
  
  //! Toggle fullscreen playback
  void toggleFullscreen();

  //! Deletes a group from playlist
  void deleteItem();
    
  void updateSelectedItems();
      
  void showAbout();

  void openProjectWebsite();

  void saveScreenshot();

  void updateSettings();
  
  void handleKeyPress(QKeyEvent* key);

  void checkNewVersion();

  // Show the open file dialog
  void showFileOpenDialog();

  void resetWindowLayout();

private slots:
  //! Timeout function for playback timer
  //void newFrameTimeout();

  void openRecentFile();
  
protected:

  virtual void keyPressEvent( QKeyEvent * event );

private:

  /// Return the primary and secondary playlist item that is currently selected
  playlistItem* selectedPrimaryPlaylistItem();
  playlistItem* selectedSecondaryPlaylistItem();

  /// Get the width/height for the current frameSize selection (in frameSizeComboBox)
  void convertFrameSizeComboBoxIndexToSize(int *width, int*height);
  
  SettingsWindow p_settingswindow;

  void createMenusAndActions();
  void updateRecentFileActions();

  // This window is shown for seperate windows mode. The main central splitViewWidget goes in here in this case.
  SeparateWindow separateViewWindow;

  // The video cache and the thread in which it is running
  videoCache *cache;

  QAction* openYUVFileAction;
  QAction* savePlaylistAction;
  QAction* addTextAction;
  QAction* addDifferenceAction;
  QAction* addOverlayAction;
  QAction* saveScreenshotAction;
  QAction* showSettingsAction;
  QAction* deleteItemAction;

  QAction* zoomToStandardAction;
  QAction* zoomToFitAction;
  QAction* zoomInAction;
  QAction* zoomOutAction;

  QAction* togglePlaylistAction;
  QAction* toggleStatisticsAction;
  QAction* toggleDisplayOptionsAction;
  QAction* toggleFileInfoAction;
  QAction* togglePropertiesAction;
  QAction* toggleControlsAction;
  QAction* toggleFullscreenAction;
  QAction* enableSingleWindowModeAction;
  QAction* enableSeparateWindowModeAction;

  QAction* playPauseAction;
  QAction* nextItemAction;
  QAction* previousItemAction;
  QAction* nextFrameAction;
  QAction* previousFrameAction;

  QAction *aboutAction;
  QAction *bugReportAction;
  QAction *featureRequestAction;
  QAction *checkNewVersionAction;
  QAction *resetWindowLayoutAction;

  QAction *recentFileActs[MAX_RECENT_FILES];

  QString strippedName(const QString &fullFileName);
};

#endif // MAINWINDOW_H
