#ifndef OPENGLPROCESSOR_H
#define OPENGLPROCESSOR_H

#include <QObject>
#include <QOffscreenSurface>
#include <QOpenGLContext>

#include "../decodercache.h"
#include "node/dependency.h"
#include "openglframebuffer.h"
#include "openglshadercache.h"
#include "render/videoparams.h"

class OpenGLWorker : public QObject {
  Q_OBJECT
public:
  OpenGLWorker(QOpenGLContext* share_ctx, OpenGLShaderCache* shader_cache, DecoderCache* decoder_cache, QObject* parent = nullptr);

  virtual ~OpenGLWorker() override;

  Q_DISABLE_COPY_MOVE(OpenGLWorker)

  bool IsStarted();

  void SetParameters(const VideoRenderingParams& video_params);

  /**
   * @brief Initialize OpenGL instance in whatever thread this object is a part of
   *
   * This function creates a context (shared with share_ctx provided in the constructor) as well as various other
   * OpenGL thread-specific objects necessary for rendering. This function should only ever be called from the main
   * thread (i.e. the thread where share_ctx is current on) but AFTER this object has been pushed to its thread with
   * moveToThread(). If this function is called from a different thread, it could fail or even segfault on some
   * platforms.
   *
   * The reason this function must be called in the main thread (rather than initializing asynchronously in a separate
   * thread) is because different platforms have different rules about creating a share context with a context that
   * is still "current" in another thread. While some implementations do allow this, Windows OpenGL (wgl) explicitly
   * forbids it and other platforms/drivers will segfault attempting it. While we can obviously call "doneCurrent", I
   * haven't found any reliable way to prevent the main thread from making it current again before initialization is
   * complete other than blocking it entirely.
   *
   * To get around this, we create all share contexts in the main thread and then move them to the other thread
   * afterwards (which is completely legal). While annoying, this gets around the issue listed above by both preventing
   * the main thread from using the context during initialization and preventing more than one shared context being made
   * at the same time (which may or may not actually make a difference).
   */
  void Init();

  bool IsAvailable();

public slots:
  void Close();

  void Render(NodeDependency path);

  void RenderAsSibling(NodeDependency dep);

  //void Download();

signals:
  void RequestSibling(NodeDependency path);

  void CompletedFrame(NodeDependency path);

private:
  void ProcessNode();

  void UpdateViewportFromParams();

  Node* ValidateBlock(Node* n, const rational& time);

  QList<NodeInput*> ProcessNodeInputsForTime(Node* n, const TimeRange& time);

  OpenGLTexturePtr RunNodeAsShader(Node *node, OpenGLShaderPtr shader);

  QOpenGLContext* share_ctx_;

  QOpenGLContext* ctx_;
  QOffscreenSurface surface_;

  QOpenGLFunctions* functions_;

  OpenGLFramebuffer buffer_;

  VideoRenderingParams video_params_;

  OpenGLShaderCache* shader_cache_;

  DecoderCache* decoder_cache_;

  QAtomicInt working_;

private slots:
  void FinishInit();

};

#endif // OPENGLPROCESSOR_H