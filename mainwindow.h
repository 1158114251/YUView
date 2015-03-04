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
#include <QTime>
#include <QTreeWidget>
#include <QMouseEvent>
#include <QTreeWidgetItem>
#include <QTimer>
#include <QDesktopWidget>
#include <QKeyEvent>

#include "settingswindow.h"
#include "edittextdialog.h"
#include "playlisttreewidget.h"

class PlaylistItem;

#include "displaywidget.h"

typedef enum {
    RepeatModeOff,
    RepeatModeOne,
    RepeatModeAll
} RepeatMode;

namespace Ui {
    class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

    void keyPressEvent( QKeyEvent * event );
    //void moveEvent ( QMoveEvent * event );
    void closeEvent(QCloseEvent *event);
    //void resizeEvent(QResizeEvent *event);

private:
    PlaylistTreeWidget *p_playlistWidget;
    Ui::MainWindow *ui;

    QTimer *p_playTimer;
    int p_currentFrame;

    QTimer *p_heartbeatTimer;

    QIcon p_playIcon;
    QIcon p_pauseIcon;
    QIcon p_repeatOffIcon;
    QIcon p_repeatAllIcon;
    QIcon p_repeatOneIcon;

    RepeatMode p_repeatMode;
    int p_numFrames;

    QMessageBox *p_msg;
    QTime p_lastHeartbeatTime;
    int p_FPSCounter;

public:
    //! loads a list of yuv/csv files
    void loadFiles(QStringList files);

    void loadPlaylistFile(QString filePath);

    bool isPlaylistItemSelected() { return selectedPrimaryPlaylistItem() != NULL; }
    void showContextMenu(QTreeWidgetItem* item, const QPoint& globalPos);



public slots:
    //! Toggle fullscreen playback
    void toggleFullscreen();

    //! Shows the file open dialog and loads all selected Files
    void openFile();

    //! Shows file open dialog, loads statistics file and associates it with current selectYUV
    void openStatsFile();

    //! Adds a new text frame
    void addTextFrame();

    void savePlaylistToFile();

    //! Starts playback of selected video file
    void play();

    //! Pauses playback of selected video file
    void pause();

    //! Toggles playback of selected video file
    void togglePlayback();

    //! Stops playback of selected video file
    void stop();

    //! Toggle playback in endless loop
    void toggleRepeat();

    //! Deletes a group from playlist
    void deleteItem();

    //! update parameters of regular overlay grid
    void updateGrid();

    void updateSelectedItems();

    //! Select a Stats Type and update GUI
    void setSelectedStats();

	//! Slot for updating the opacity of the current selected stats type (via items model)
    void updateStatsOpacity(int val);

	//! Slot for updating the grid visibility of the current selected stats type (via items model)
    void updateStatsGrid(bool val);

    //! set current frame for playback
    void setCurrentFrame( int frame, bool forceRefresh = false );

    //! enables the playback controls
    void setControlsEnabled(bool flag);

    //! updates the YUV information GUI elements from the current Renderobject
    void updateMetaInfo();

    //! updates the Playback controls to fit the current YUV settings
    void refreshPlaybackWidgets();

    //! update selection of frame size ComboBox
    void updateFrameSizeComboBoxSelection();

    //! update selection of color format ComboBox
    void updateColorFormatComboBoxSelection(PlaylistItem *selectedItem);

    //! this event is called when the playback-timer is triggered. It will paint the next frame
    void frameTimerEvent();

    void heartbeatTimerEvent();

    void showAbout();

    void bugreport();

    void saveScreenshot();

    void updateSettings();

    void editTextFrame();


private slots:
    //! Timeout function for playback timer
    //void newFrameTimeout();

    void statsTypesChanged();

    void on_interpolationComboBox_currentIndexChanged(int index);
    void on_pixelFormatComboBox_currentIndexChanged(int index);
    void on_sizeComboBox_currentIndexChanged(int index);
    void onCustomContextMenu(const QPoint &point);
    void onItemDoubleClicked(QTreeWidgetItem* item, int row);

    void openRecentFile();

    void on_SplitviewCheckBox_stateChanged(int arg1);

    void on_LumaScaleSpinBox_valueChanged(int index);

    void on_ChormaScaleSpinBox_valueChanged(int index);

    void on_LumaOffsetSpinBox_valueChanged(int arg1);

    void on_ChormaOffsetSpinBox_valueChanged(int arg1);

    void on_LumaInvertCheckBox_toggled(bool checked);

    void on_ChromaInvertCheckBox_toggled(bool checked);

    void on_ColorComponentsComboBox_currentIndexChanged(int index);

    void selectNextItem();
    void selectPreviousItem();
    void nextFrame() { setCurrentFrame( p_currentFrame+1 ); }
    void previousFrame() { setCurrentFrame( p_currentFrame-1 ); }


private:
    int findMaxNumFrames();
    PlaylistItem* selectedPrimaryPlaylistItem();
    PlaylistItem* selectedSecondaryPlaylistItem();

    static QVector<StatisticsRenderItem> p_emptyTypes;
    SettingsWindow p_settingswindow;

    void createMenusAndActions();
    void updateRecentFileActions();

    QAction* openYUVFileAction;
    QAction* openStatisticsFileAction;
    QAction* savePlaylistAction;
    QAction* addTextAction;
    QAction* saveScreenshotAction;
    QAction* showSettingsAction;

    QAction* zoomToStandardAction;
    QAction* zoomToFitAction;
    QAction* zoomInAction;
    QAction* zoomOutAction;

    QAction* togglePlaylistAction;
    QAction* toggleStatisticsAction;
    QAction* toggleFileOptionsAction;
    QAction* toggleDisplayOptionsActions;
    QAction* toggleControlsAction;
    QAction* toggleFullscreenAction;

    QAction* playPauseAction;
    QAction* nextItemAction;
    QAction* previousItemAction;
    QAction* nextFrameAction;
    QAction* previousFrameAction;

    QAction *aboutAction;
    QAction *bugReportAction;
    QAction *featureRequestAction;

    enum { MaxRecentFiles = 5 };
    QAction *recentFileActs[MaxRecentFiles];

    QString strippedName(const QString &fullFileName);
};

#endif // MAINWINDOW_H
