<?php
## ST-500 Renewal (2022.02.23 Start. dev@suntech.asia & hamani@naver.com)

$userId = 'dev@suntech.asia'; // 임시사용.
$userName = 'dh.kim'; // 임시사용.
?>

<style>
</style>

<nav class="navbar navbar-expand-lg navbar-light bg-light py-3 shadow-lg sticky-top">
  <div class="container-fluid">
    <a href="index.php" class="navbar-brand align-middle">
      <h3 class="m-0">ST-500 TIME LOCK SYSTEM</h3>
    </a>
    <!-- hamburger button -->
    <button class="navbar-toggler" type="button" data-bs-toggle="collapse" data-bs-target="#navbarMenu"
      aria-controls="navbarMenu" aria-expanded="false" aria-label="Toggle navigation">
      <span class="navbar-toggler-icon"></span>
    </button>

    <!-- menu -->
    <div class="collapse navbar-collapse" id="navbarMenu">
      <ul class="navbar-nav ms-auto mb-2 mb-lg-0">
        <li class="nav-item">
          <a href="setting_device.php" class="nav-link">
				<svg xmlns="http://www.w3.org/2000/svg" width="18" height="18" viewBox="0 0 24 24" fill="none"
					stroke="currentColor" stroke-width="2" stroke-linecap="round" stroke-linejoin="round"
					class="align-middle">
					<line x1="4" y1="21" x2="4" y2="14"></line>
					<line x1="4" y1="10" x2="4" y2="3"></line>
					<line x1="12" y1="21" x2="12" y2="12"></line>
					<line x1="12" y1="8" x2="12" y2="3"></line>
					<line x1="20" y1="21" x2="20" y2="16"></line>
					<line x1="20" y1="12" x2="20" y2="3"></line>
					<line x1="1" y1="14" x2="7" y2="14"></line>
					<line x1="9" y1="8" x2="15" y2="8"></line>
					<line x1="17" y1="16" x2="23" y2="16"></line>
				</svg> DEVICE</a>
        </li>

        <li class="nav-item">
          <a href="parameter.php" class="nav-link">
				<svg xmlns="http://www.w3.org/2000/svg" width="18" height="18" viewBox="0 0 24 24" fill="none"
					stroke="currentColor" stroke-width="2" stroke-linecap="round" stroke-linejoin="round"
					class="align-middle">
					<circle cx="12" cy="12" r="3"></circle>
					<path
						d="M19.4 15a1.65 1.65 0 0 0 .33 1.82l.06.06a2 2 0 0 1 0 2.83 2 2 0 0 1-2.83 0l-.06-.06a1.65 1.65 0 0 0-1.82-.33 1.65 1.65 0 0 0-1 1.51V21a2 2 0 0 1-2 2 2 2 0 0 1-2-2v-.09A1.65 1.65 0 0 0 9 19.4a1.65 1.65 0 0 0-1.82.33l-.06.06a2 2 0 0 1-2.83 0 2 2 0 0 1 0-2.83l.06-.06a1.65 1.65 0 0 0 .33-1.82 1.65 1.65 0 0 0-1.51-1H3a2 2 0 0 1-2-2 2 2 0 0 1 2-2h.09A1.65 1.65 0 0 0 4.6 9a1.65 1.65 0 0 0-.33-1.82l-.06-.06a2 2 0 0 1 0-2.83 2 2 0 0 1 2.83 0l.06.06a1.65 1.65 0 0 0 1.82.33H9a1.65 1.65 0 0 0 1-1.51V3a2 2 0 0 1 2-2 2 2 0 0 1 2 2v.09a1.65 1.65 0 0 0 1 1.51 1.65 1.65 0 0 0 1.82-.33l.06-.06a2 2 0 0 1 2.83 0 2 2 0 0 1 0 2.83l-.06.06a1.65 1.65 0 0 0-.33 1.82V9a1.65 1.65 0 0 0 1.51 1H21a2 2 0 0 1 2 2 2 2 0 0 1-2 2h-.09a1.65 1.65 0 0 0-1.51 1z">
					</path>
				</svg> PARAMETER</a>
        </li>

        <li class="nav-item">
          <a href="log.php" class="nav-link">
				<svg xmlns="http://www.w3.org/2000/svg" width="18" height="18" viewBox="0 0 24 24" fill="none"
					stroke="currentColor" stroke-width="2" stroke-linecap="round" stroke-linejoin="round"
					class="align-middle">
					<line x1="8" y1="6" x2="21" y2="6"></line>
					<line x1="8" y1="12" x2="21" y2="12"></line>
					<line x1="8" y1="18" x2="21" y2="18"></line>
					<line x1="3" y1="6" x2="3.01" y2="6"></line>
					<line x1="3" y1="12" x2="3.01" y2="12"></line>
					<line x1="3" y1="18" x2="3.01" y2="18"></line>
				</svg> LOG</a>
        </li>

        <!-- user info -->
        <!-- <li class="nav-item dropdown">
          <a class="nav-icon dropdown-toggle d-inline-block d-sm-none" href="#" data-bs-toggle="dropdown">
            <i class="align-middle" data-feather="settings"></i>
          </a>
          <a class="nav-link dropdown-toggle d-none d-sm-inline-block" href="#" data-bs-toggle="dropdown">
            <i class="align-middle fas fa-fw fa-user-circle me-0 fa-lg" aria-hidden="true"></i> <span
              class="text-dark"><?php echo $userName;?></span>
          </a> -->
          <!-- LOG OUT -->
          <!-- <div class="dropdown-menu dropdown-menu-end">
            <a class="dropdown-item" href="#">
              <i class="align-middle me-1" data-feather="user"></i> <?php echo $userId;?>
            </a>
            <div class="dropdown-divider"></div>
            <a class="dropdown-item" href="#"><strong>Log out</strong></a>
          </div>
        </li> -->
      </ul>
    </div>
  </div>
</nav>