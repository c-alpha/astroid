# include <iostream>
# include <fstream>

# include <gtkmm.h>
# include <webkit/webkit.h>

# include "thread_view.hh"
# include "message_thread.hh"


using namespace std;

namespace Astroid {
  bool ThreadView::theme_loaded = false;
  const char * ThreadView::thread_view_html_f = "ui/thread-view.html";
  const char * ThreadView::thread_view_css_f  = "ui/thread-view.css";
  ustring ThreadView::thread_view_html;
  ustring ThreadView::thread_view_css;


  ThreadView::ThreadView () {
    tab_widget = new Gtk::Label (thread_id);

    pack_start (scroll, true, true, 5);

    /* set up webkit web view (using C api) */
    webview = WEBKIT_WEB_VIEW (webkit_web_view_new ());

    websettings = WEBKIT_WEB_SETTINGS (webkit_web_settings_new ());
    /* use g_object_set / g_object_get
    webkit_settings_set_auto_load_images (websettings, false);
    webkit_settings_set_enable_html5_database (websettings, false);
    webkit_settings_set_enable_html5_local_storage (websettings, false);
    webkit_settings_set_enable_hyperlink_auditing (websettings, false);
    webkit_settings_set_enable_java (websettings, false);
    webkit_settings_set_enable_javascript (websettings, false);
    webkit_settings_set_enable_plugins (websettings, false);
    webkit_settings_set_enable_xss_auditor (websettings, false);


    //webkit_settings_set_enable_spatial_navigation (websettings, true);
    webkit_settings_set_media_playback_requires_user_gesture (websettings, true);
    webkit_settings_set_enable_webaudio (websettings, true);
    webkit_settings_set_enable_webgl (websettings, true);
    webkit_settings_set_enable_private_browsing (websettings, true);
    webkit_settings_set_enable_fullscreen (websettings, true);
    */

    webkit_web_view_set_settings (webview, websettings);


    gtk_container_add (GTK_CONTAINER (scroll.gobj()), GTK_WIDGET(webview));


    scroll.show_all ();

    /* load css, html and DOM objects */
    if (!theme_loaded) {
      ifstream tv_html_f (thread_view_html_f);
      istreambuf_iterator<char> eos; // default is eos
      istreambuf_iterator<char> tv_iit (tv_html_f);

      thread_view_html.append (tv_iit, eos);
      tv_html_f.close ();

      ifstream tv_css_f (thread_view_css_f);
      istreambuf_iterator<char> tv_css_iit (tv_css_f);
      thread_view_css.append (tv_css_iit, eos);
      tv_css_f.close ();

      theme_loaded = true;
    }

    g_signal_connect (webview, "notify::load-status",
        G_CALLBACK(ThreadView_on_load_changed),
        (gpointer) this );

    webkit_web_view_load_html_string (webview, thread_view_html.c_str (), "/tmp/");

  }

  ThreadView::~ThreadView () {
    g_object_unref (webview);
    g_object_unref (websettings);
  }

  /* is this callback setup safe?
   *
   * http://stackoverflow.com/questions/2068022/in-c-is-it-safe-portable-to-use-static-member-function-pointer-for-c-api-call
   *
   * http://gtk.10911.n7.nabble.com/Using-g-signal-connect-in-class-td57137.html
   *
   * to be portable we have to use a free function declared extern "C". a
   * static member function * is likely to work at least on gcc/g++, but not
   * necessarily elsewhere.
   *
   */

  extern "C" bool ThreadView_on_load_changed (
      GtkWidget *       w,
      GParamSpec *      p,
      gpointer          data )
  {
    return ((ThreadView *) data)->on_load_changed (w, p);
  }

  bool ThreadView::on_load_changed (
      GtkWidget *       w,
      GParamSpec *      p)
  {
    WebKitLoadStatus ev = webkit_web_view_get_load_status (webview);
    cout << "tv: on_load_changed: " << ev << endl;
    switch (ev) {
      case WEBKIT_LOAD_FINISHED:
        cout << "tv: load finished." << endl;
        {
          /* load css style */
          GError *err = NULL;
          WebKitDOMDocument *d = webkit_web_view_get_dom_document (webview);
          WebKitDOMElement  *e = webkit_dom_document_create_element (d, STYLE_NAME, &err);

          WebKitDOMText *t = webkit_dom_document_create_text_node
            (d, thread_view_css.c_str());

          webkit_dom_node_append_child (WEBKIT_DOM_NODE(e), WEBKIT_DOM_NODE(t), (err = NULL, &err));

          WebKitDOMHTMLHeadElement * head = webkit_dom_document_get_head (d);
          webkit_dom_node_append_child (WEBKIT_DOM_NODE(head), WEBKIT_DOM_NODE(e), (err = NULL, &err));


          g_object_unref (d);
          g_object_unref (e);
          g_object_unref (t);
          g_object_unref (head);

          /* unlock message loading */

        }
        break;
    }

    return true;
  }

  void ThreadView::load_thread (ustring _thread_id) {
    thread_id = _thread_id;

    ((Gtk::Label*) tab_widget)->set_text (thread_id);

    mthread = new MessageThread (thread_id);
    mthread->load_messages ();
  }

  void ThreadView::render () {
    //webkit_web_view_load_html (webview, mthread->messages[0]->body().c_str(), "/tmp/");




  }

  void ThreadView::grab_modal () {
    //gtk_grab_add (GTK_WIDGET (webview));
    //gtk_widget_grab_focus (GTK_WIDGET (webview));
  }

  void ThreadView::release_modal () {
    //gtk_grab_remove (GTK_WIDGET (webview));
  }

}

