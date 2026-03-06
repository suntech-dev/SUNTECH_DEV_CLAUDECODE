/**
 * Stats Card Toggle Functionality
 * Common toggle function for all manage pages
 */

export function initStatsToggle() {
  const toggleStatsBtn = document.getElementById('toggleStatsBtn');
  const statsGrid = document.getElementById('statsGrid');

  if (!toggleStatsBtn || !statsGrid) {
    return; // Exit if elements not found
  }

  toggleStatsBtn.addEventListener('click', function() {
    if (statsGrid.classList.contains('hidden')) {
      // Show stats
      statsGrid.classList.remove('hidden');
      toggleStatsBtn.textContent = '📊 Hide Stats';
    } else {
      // Hide stats
      statsGrid.classList.add('hidden');
      toggleStatsBtn.textContent = '📊 Show Stats';
    }
  });
}
