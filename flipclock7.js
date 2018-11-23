/**
 * @file
 * Behavior for flipclock7.
 *
 * Initializes the jQuery Countdown Timer.
 */

(function ($) {
  Drupal.behaviors.flipclock7 = {
    attach: function (context, settings) {
      var i = 0;
      $.each($('.flipclock7', context), function (index, element) {
        var countdown = $('.flipclock7-countdown', $(this));
        var settings = Drupal.settings.flipclock7[countdown.attr('id')];
        var ts = new Date(settings['time'] * 1000);
        var left = Math.floor((ts - (new Date())) / 1000);
        if (left < 0) {
          left = 0;
        }
        var clock = $('.flipclock7-countdown', $(this)).FlipClock( left, {
          clockFace: 'DailyCounter',
          countdown: true,
          language: settings['language'],
        });
      });
    }
  };
})(jQuery);
